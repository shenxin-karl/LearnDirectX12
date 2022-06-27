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

void AssimpLoader::parse(std::vector<ALMesh> &meshs) {
	assert(isLoad());
	for (size_t i = 0; i < _pScene->mNumMeshes; ++i) {
		const aiMesh *pAiMesh = _pScene->mMeshes[i];
		ALMesh mesh;
		processVertices(mesh.vertices, pAiMesh);
		processTriangles(mesh.indices, pAiMesh);
		meshs.push_back(std::move(mesh));
	}
}



void AssimpLoader::parse(std::vector<ALSkinnedMesh> &meshs) {
	assert(isLoad());
	std::vector<BoneInfo> boneInfos;
	for (size_t i = 0; i < _pScene->mNumMeshes; ++i) {
		const aiMesh *pAiMesh = _pScene->mMeshes[i];
		ALSkinnedMesh mesh;
		processSkinnedVertices(mesh.vertices, pAiMesh);
		processTriangles(mesh.indices, pAiMesh);
		//processBoneOffsets();
		meshs.push_back(std::move(mesh));
	}
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

void AssimpLoader::processTriangles(std::vector<uint16_t> &indices, const aiMesh *pAiMesh) {
	size_t numIndices = 0;
	for (size_t i = 0; i < pAiMesh->mNumFaces; ++i)
		numIndices += pAiMesh->mFaces[i].mNumIndices;

	indices.resize(numIndices);
	for (size_t i = 0; i < pAiMesh->mNumFaces; ++i) {
		const aiFace &face = pAiMesh->mFaces[i];
		copy_n(face.mIndices, face.mNumIndices, indices.begin());
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
		}
		else {
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
		}
		else {
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
		boneInfo.boneIndexMap[boneName] = i;
		boneInfo.boneNames.push_back(pAiBone->mName.C_Str());
		boneInfo.boneOffsets[i] = convertFloat4x4(pAiBone->mOffsetMatrix);
	}
}

void AssimpLoader::processBoneHierarchyAndAnimation(std::vector<BoneInfo> boneInfos, std::vector<ALSkinnedMesh> &meshs) {
	std::vector<AnimationClip> clips;
	std::unordered_map<std::string, std::string> boneParentName;

	std::stack<const aiNode *> nodeStack;
	nodeStack.push(_pScene->mRootNode);
	while (!nodeStack.empty()) {
		const aiNode *pAiNode = nodeStack.top();
		const aiNode *pParent = pAiNode->mParent;
		boneParentName[pAiNode->mName.C_Str()] = (pParent != nullptr) ? pParent->mName.C_Str() : "";
		nodeStack.pop();
		for (size_t i = 0; i < pAiNode->mNumChildren; ++i)
			nodeStack.push(pAiNode->mChildren[i]);
	}

	for (size_t i = 0; i < boneInfos.size(); ++i) {
		BoneInfo &boneInfo = boneInfos[i];
		SkinnedData &skinnedData = meshs[i].skinnedData;
		size_t boneSize = boneInfo.boneOffsets.size();
		skinnedData._boneHierarchy.resize(boneSize);
		skinnedData._boneOffsets.resize(boneSize);
		for (size_t j = 0; j < boneInfo.boneOffsets.size(); ++i) {
			size_t parentIndex = 0;
			const auto &boneName = boneInfo.boneNames[i];
			auto iter = boneParentName.find(boneName);
			if (iter != boneParentName.end()) {
				const auto &parentName = iter->second;
				auto boneNameIter = boneInfo.boneIndexMap.find(parentName);
				if (boneNameIter != boneInfo.boneIndexMap.end())
					parentIndex = boneNameIter->second;
			}

			skinnedData._boneHierarchy[j] = parentIndex;
			skinnedData._boneOffsets[j] = boneInfo.boneOffsets[j];
		}
	}

	for (size_t i = 0; i < _pScene->mNumAnimations; ++i) {
		
	}
}

}
