#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <D3D/AssimpLoader/AssimpLoader.h>
#include <D3D/Tool/Mesh.h>
#include <stack>

namespace d3d {


AssimpLoader::AssimpLoader(const std::string &fileName, bool bLoad) : _fileName(fileName) {
	if (!bLoad)
		return;
	load();
}

bool AssimpLoader::load() {
	if (_isLoad)
		return true;

	int flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_FixInfacingNormals;
	flag |= aiProcess_CalcTangentSpace;
	flag |= aiProcess_ConvertToLeftHanded;
	_pScene = _importer.ReadFile(_fileName, flag);
	if (_pScene == nullptr || _pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || _pScene->mRootNode == nullptr)
		return false;

	_isLoad = true;
	return true;
}

bool AssimpLoader::isLoad() const {
	return _isLoad;
}

std::vector<AssimpLoader::ALMesh> AssimpLoader::parseMesh() const {
	assert(isLoad());
	std::vector<ALMesh> meshs;
	for (size_t i = 0; i < _pScene->mNumMeshes; ++i) {
		ALMesh mesh;
		const aiMesh *pAiMesh = _pScene->mMeshes[i];
		processVertices(mesh.vertices, pAiMesh);
		processTriangles(mesh.indices, pAiMesh);
		meshs.push_back(std::move(mesh));
	}
	return meshs;
}

std::vector<AssimpLoader::ALSkinnedMesh> AssimpLoader::parseSkinnedMesh() const {
	assert(isLoad());
	std::vector<ALSkinnedMesh> meshs;
	std::vector<BoneInfo> boneInfos;
	boneInfos.resize(_pScene->mNumMeshes);
	for (size_t i = 0; i < _pScene->mNumMeshes; ++i) {
		ALSkinnedMesh mesh;
		const aiMesh *pAiMesh = _pScene->mMeshes[i];
		processSkinnedVertices(mesh.vertices, pAiMesh);
		processTriangles(mesh.indices, pAiMesh);
		processBoneOffsets(boneInfos[i], pAiMesh);
		meshs.push_back(std::move(mesh));
	}
	processBoneHierarchyAndAnimation(meshs, boneInfos);
	return meshs;
}

float4x4 AssimpLoader::convertFloat4x4(const aiMatrix4x4 &m) {
	float4x4 d;
	auto &s = m;
	d(0, 0) = s.a1; d(0, 1) = s.a2; d(0, 2) = s.a3; d(0, 3) = s.a4;
	d(1, 0) = s.b1; d(1, 1) = s.b2; d(1, 2) = s.b3; d(1, 3) = s.b4;
	d(2, 0) = s.c1; d(2, 1) = s.c2; d(2, 2) = s.c3; d(2, 3) = s.c4;
	d(3, 0) = s.d1; d(3, 1) = s.d2; d(3, 2) = s.d3; d(3, 3) = s.d4;
	return d;
}

float3 AssimpLoader::convertFloat3(const aiVector3D &v) {
	return { v.x, v.y, v.z };
}

float4 AssimpLoader::convertFloat4(const aiQuaternion &q) {
	return { q.x, q.y, q.z, q.w };
}

void AssimpLoader::processTriangles(std::vector<uint16_t> &indices, const aiMesh *pAiMesh) {
	size_t numIndices = 0;
	for (size_t i = 0; i < pAiMesh->mNumFaces; ++i)
		numIndices += pAiMesh->mFaces[i].mNumIndices;

	indices.resize(numIndices);
	auto pDesc = indices.begin();
	for (size_t i = 0; i < pAiMesh->mNumFaces; ++i) {
		const aiFace &face = pAiMesh->mFaces[i];
		copy_n(face.mIndices, face.mNumIndices, pDesc);
		pDesc += face.mNumIndices;
	}
}

void AssimpLoader::processVertices(std::vector<com::Vertex> &vertices, const aiMesh *pAiMesh) {
	vertices.resize(pAiMesh->mNumVertices);
	for (size_t i = 0; i < pAiMesh->mNumVertices; ++i) {
		com::Vertex &vertex = vertices[i];
		vertex.position.x = pAiMesh->mVertices[i].x;
		vertex.position.y = pAiMesh->mVertices[i].y;
		vertex.position.z = pAiMesh->mVertices[i].z;
		vertex.normal.x = pAiMesh->mNormals[i].x;
		vertex.normal.y = pAiMesh->mNormals[i].y;
		vertex.normal.z = pAiMesh->mNormals[i].z;
		if (pAiMesh->mTextureCoords[0]) {
			vertex.texcoord.x = pAiMesh->mTextureCoords[0][i].x;
			vertex.texcoord.y = pAiMesh->mTextureCoords[0][i].y;
			vertex.tangent.x = pAiMesh->mTangents[i].x;
			vertex.tangent.y = pAiMesh->mTangents[i].y;
			vertex.tangent.z = pAiMesh->mTangents[i].z;
		} else {
			vertex.texcoord = float2::identity();
			vertex.tangent = float3::identity();
		}
	}
}

void AssimpLoader::processSkinnedVertices(std::vector<SkinnedVertex> &vertices, const aiMesh *pAiMesh) {
	vertices.resize(pAiMesh->mNumVertices);
	for (size_t i = 0; i < pAiMesh->mNumVertices; ++i) {
		SkinnedVertex &vertex = vertices[i];
		vertex.position.x = pAiMesh->mVertices[i].x;
		vertex.position.y = pAiMesh->mVertices[i].y;
		vertex.position.z = pAiMesh->mVertices[i].z;
		vertex.normal.x = pAiMesh->mNormals[i].x;
		vertex.normal.y = pAiMesh->mNormals[i].y;
		vertex.normal.z = pAiMesh->mNormals[i].z;
		if (pAiMesh->mTextureCoords[0]) {
			vertex.texcoord.x = pAiMesh->mTextureCoords[0][i].x;
			vertex.texcoord.y = pAiMesh->mTextureCoords[0][i].y;
			vertex.tangent.x = pAiMesh->mTangents[i].x;
			vertex.tangent.y = pAiMesh->mTangents[i].y;
			vertex.tangent.z = pAiMesh->mTangents[i].z;
		} else {
			vertex.texcoord = float2::identity();
			vertex.tangent = float3::identity();
		}
		vertex.boneIndices[0] = 0;
		vertex.boneIndices[1] = 0;
		vertex.boneIndices[2] = 0;
		vertex.boneIndices[3] = 0;
		vertex.boneWeights = float3{ 0.f };
	}
}

void AssimpLoader::processBoneOffsets(BoneInfo &boneInfo, const aiMesh *pAiMesh) {
	boneInfo.boneOffsets.resize(pAiMesh->mNumBones);
	boneInfo.boneNames.resize(pAiMesh->mNumBones);
	for (size_t i = 0; i < pAiMesh->mNumBones; ++i) {
		const aiBone *pAiBone = pAiMesh->mBones[i];
		std::string boneName{ pAiBone->mName.C_Str() };
		boneInfo.boneIndexMap[boneName] = static_cast<uint8_t>(i);
		boneInfo.boneNames.push_back(pAiBone->mName.C_Str());
		boneInfo.boneOffsets[i] = convertFloat4x4(pAiBone->mOffsetMatrix);
	}
}


void AssimpLoader::processBoneHierarchyAndAnimation(std::vector<ALSkinnedMesh> &meshs, const std::vector<BoneInfo> &boneInfos) const {
	if (!_pScene->HasAnimations())
		return;

	std::vector<size_t> boneHierarchy;
	std::vector<std::string> boneNames;
	std::unordered_map<std::string, AiNodeInfo> boneInfoMap;
	std::stack<const aiNode *> aiNodeStack;

	// 计算好层级关系
	aiNodeStack.push(_pScene->mRootNode);
	while (!aiNodeStack.empty()) {
		const aiNode *pAiNode = aiNodeStack.top();
		std::string boneName{ pAiNode->mName.C_Str(), pAiNode->mName.length };
		boneInfoMap[boneName] = { boneNames.size(), convertFloat4x4(pAiNode->mTransformation) };
		boneNames.emplace_back(boneName);
		if (pAiNode->mParent != nullptr) {
			auto iter = boneInfoMap.find(pAiNode->mParent->mName.C_Str());
			assert(iter != boneInfoMap.end());
			boneHierarchy.push_back(iter->second.index);
		} else {
			boneHierarchy.push_back(-1);
		}
		aiNodeStack.pop();
		for (size_t i = 0; i < pAiNode->mNumChildren; ++i)
			aiNodeStack.push(pAiNode->mChildren[i]);
	}


	// 计算好 bone offsets 和赋值 bone hierarchy
	size_t nodeSize = boneNames.size();
	for (size_t i = 0; i < meshs.size(); ++i) {
		SkinnedData &skinnedData = meshs[i].skinnedData;
		const BoneInfo &boneInfo = boneInfos[i];
		skinnedData._boneHierarchy = boneHierarchy;
		skinnedData._boneOffsets.resize(nodeSize);
		for (size_t j = 0; j < nodeSize; ++j) {
			const std::string &boneName = boneNames[j];
			auto iter = boneInfo.boneIndexMap.find(boneName);
			if (iter != boneInfo.boneIndexMap.end())
				skinnedData._boneOffsets[j] = boneInfo.boneOffsets[iter->second];
			else
				skinnedData._boneOffsets[j] = float4x4::identity();
		}
	}

	std::unordered_map<std::string, const aiNodeAnim *> aiNodeAnimMap;
	auto processAnimations = [&](AnimationClip &animationClip, float ticksPerSecond) {
		if (ticksPerSecond < 0.0001f)
			ticksPerSecond = 1.f;
		else
			ticksPerSecond = 1.f / ticksPerSecond;

		for (size_t i = 0; i < nodeSize; ++i) {
			const std::string &boneName = boneNames[i];
			BoneAnimation &boneAnimation = animationClip.boneAnimations[i];

			// 这个动画下的这骨骼拥有关键帧
			if (auto iter = aiNodeAnimMap.find(boneName); iter != aiNodeAnimMap.end()) {
				const aiNodeAnim *pKeyframeInfo = iter->second;
				assert(pKeyframeInfo->mNumPositionKeys > 0);
				assert(pKeyframeInfo->mNumPositionKeys == pKeyframeInfo->mNumRotationKeys);
				assert(pKeyframeInfo->mNumRotationKeys == pKeyframeInfo->mNumScalingKeys);
				for (size_t j = 0; j < pKeyframeInfo->mNumPositionKeys; ++j) {
					Keyframe keyframe;
					keyframe.timePoint = static_cast<float>(pKeyframeInfo->mPositionKeys[j].mTime) * ticksPerSecond;
					keyframe.translation = convertFloat3(pKeyframeInfo->mPositionKeys[j].mValue);
					keyframe.scale = convertFloat3(pKeyframeInfo->mScalingKeys[j].mValue);
					keyframe.rotationQuat = convertFloat4(pKeyframeInfo->mRotationKeys[j].mValue);
					boneAnimation.keyframes.push_back(keyframe);
				}

			// 当前动画中的这个骨骼不需要关键帧, 那么使用 aiNode::mTransformation 构建关键帧传递
			} else {
				Matrix4 nodeTransform = Matrix4(boneInfoMap[boneName].nodeTransform);
				Vector3 scale;
				Vector3 trans;
				Quaternion rotateQuat;
				DX::XMMatrixDecompose(&scale, &trans, &rotateQuat, nodeTransform.operator DX::XMMATRIX());
				Keyframe keyframe;
				keyframe.timePoint = std::numeric_limits<float>::infinity();
				keyframe.scale = float3(scale);
				keyframe.translation = float3(trans);
				keyframe.rotationQuat = float4(rotateQuat);
				boneAnimation.keyframes.push_back(keyframe);
			}
		}
	};

	// 处理每个动画
	for (size_t i = 0; i < _pScene->mNumAnimations; ++i) {
		const aiAnimation *pAnimation = _pScene->mAnimations[i];

		// 建立动画中骨骼名称到动画数据的HashMap
		aiNodeAnimMap.clear();
		for (size_t j = 0; j < pAnimation->mNumChannels; ++j) {
			const aiNodeAnim *pAiNodeAnim = pAnimation->mChannels[j];
			aiNodeAnimMap[pAiNodeAnim->mNodeName.C_Str()] = pAiNodeAnim;
		}

		// 处理关键帧
		for (size_t j = 0; j < meshs.size(); ++j) {
			SkinnedData &skinnedData = meshs[j].skinnedData;
			AnimationClip &animationClip = skinnedData._animations[pAnimation->mName.C_Str()];
			animationClip.boneAnimations.resize(nodeSize);
			processAnimations(animationClip, static_cast<float>(pAnimation->mTicksPerSecond));
		}
	}
}

}
