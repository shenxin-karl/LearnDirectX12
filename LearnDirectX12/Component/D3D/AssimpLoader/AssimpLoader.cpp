#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <D3D/AssimpLoader/AssimpLoader.h>

#include "D3D/Tool/Mesh.h"

namespace d3d {


std::optional<std::string> AssimpLoader::load(const std::string &fileName,
	std::vector<ALMesh> &meshes, 
	std::vector<ALMaterial> &materials)
{
	int flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_FixInfacingNormals;
	flag |= aiProcess_CalcTangentSpace;
	flag |= aiProcess_ConvertToLeftHanded;
	Assimp::Importer import;
	const aiScene *pScene = import.ReadFile(fileName, flag);
	if (pScene == nullptr							||  // 场景节点为空
		pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE  || 	// 不完全加载
		pScene->mRootNode == nullptr				) 	// 根节点为空
	{
		return import.GetErrorString();
	}

	for (size_t i = 0; i < pScene->mNumMeshes; ++i) 
		processMesh(meshes, materials, pScene->mMeshes[i], pScene);
	
	return std::nullopt;
}

std::optional<std::string> AssimpLoader::load(const std::string &fileName,
	std::vector<ALSkinnedMesh> meshes, 
	std::vector<ALMaterial> &materails)
{
	int flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_FixInfacingNormals;
	flag |= aiProcess_CalcTangentSpace;
	flag |= aiProcess_ConvertToLeftHanded;
	Assimp::Importer import;
	const aiScene *pScene = import.ReadFile(fileName, flag);
	if (pScene == nullptr || 							// 场景节点为空
		pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || 	// 不完全加载
		pScene->mRootNode == nullptr) {					// 根节点为空
		return import.GetErrorString();
	}

	processNode(meshes, materails, pScene->mRootNode, pScene);
	return std::nullopt;
}

void AssimpLoader::processNode(std::vector<ALMesh> &meshes, 
	std::vector<ALMaterial> &materails, 
	aiNode *pAiNode, 
	const aiScene *pScene)
{
	for (size_t i = 0; i < pAiNode->mNumMeshes; ++i) {
		int index = pAiNode->mMeshes[i];
		aiMesh *pAiMesh = pScene->mMeshes[index];
		processMesh(meshes, materails, pAiMesh, pScene);
	}
	for (size_t i = 0; i < pAiNode->mNumChildren; ++i)
		processNode(meshes, materails, pAiNode->mChildren[i], pScene);
}

void AssimpLoader::processNode(std::vector<ALSkinnedMesh> &meshes, 
	std::vector<ALMaterial> &materails, 
	aiNode *pAiNode, 
	const aiScene *pScene)
{
	for (size_t i = 0; i < pAiNode->mNumMeshes; ++i) {
		int index = pAiNode->mMeshes[i];
		aiMesh *pAiMesh = pScene->mMeshes[index];
		processMesh(meshes, materails, pAiMesh, pScene);
	}
	for (size_t i = 0; i < pAiNode->mNumChildren; ++i)
		processNode(meshes, materails, pAiNode->mChildren[i], pScene);
}

void AssimpLoader::processMesh(std::vector<ALMesh> &meshes, 
	std::vector<ALMaterial> &materials, 
	aiMesh *pAiMesh, 
	const aiScene *pScene)
{
	ALMesh mesh;
	mesh.vertices.reserve(pAiMesh->mNumVertices);
	for (size_t i = 0; i < pAiMesh->mNumVertices; ++i) {
		com::Vertex vertex;
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
		mesh.vertices.push_back(vertex);
	}

	processTriangles(mesh.indices, pAiMesh);
	mesh.materialIndex = processMaterials(materials, pAiMesh, pScene);
}

void AssimpLoader::processMesh(std::vector<ALSkinnedMesh> &meshes, 
	std::vector<ALMaterial> &materails, 
	aiMesh *pAiMesh, 
	const aiScene *pScene)
{
	ALSkinnedMesh mesh;
	mesh.vertices.reserve(pAiMesh->mNumVertices);
	for (size_t i = 0; i < pAiMesh->mNumVertices; ++i) {
		SkinnedVertex vertex;
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
		mesh.vertices.push_back(vertex);
	}

	processTriangles(mesh.indices, pAiMesh);
	mesh.materialIndex = processMaterials(materails, pAiMesh, pScene);

	SkinnedData skinnedData;
	std::vector<float4x4> boneOffsets;
	processBone(mesh.vertices, boneOffsets, pAiMesh);

	//processBoneHierarchy();
}

void AssimpLoader::processTriangles(std::vector<uint16_t> &indices, aiMesh *pAiMesh) {
	size_t numIndices = 0;
	for (size_t i = 0; i < pAiMesh->mNumFaces; ++i)
		numIndices += pAiMesh->mFaces[i].mNumIndices;

	indices.reserve(numIndices);
	for (size_t i = 0; i < pAiMesh->mNumFaces; ++i) {
		const aiFace &face = pAiMesh->mFaces[i];
		copy_n(face.mIndices, face.mNumIndices, std::back_insert_iterator(indices));
	}
}

size_t AssimpLoader::processMaterials(std::vector<ALMaterial> &materails, aiMesh *pAiMesh, const aiScene *pScene) {
	auto getMapName = [](aiMaterial *pAiMaterial, aiTextureType type) -> std::string {
		if (pAiMaterial->GetTextureCount(type) > 0) {
			aiString str;
			pAiMaterial->GetTexture(type, 0, &str);
			return std::string{ str.data, str.length };
		} else {
			if (type == aiTextureType_NORMALS)
				return "default_nmap.dds";
			else
				return "white1x1.dds";
		}
	};


	int materialIndex = static_cast<int>(pAiMesh->mNumAnimMeshes);
	if (materialIndex >= 0) {
		ALMaterial mat;
		aiMaterial *pAiMaterial = pScene->mMaterials[materialIndex];
		mat.albedoMapName = getMapName(pAiMaterial, aiTextureType_DIFFUSE);
		mat.normalMapName = getMapName(pAiMaterial, aiTextureType_NORMALS);
		mat.metallicMapName = getMapName(pAiMaterial, aiTextureType_METALNESS);
		mat.roughnessMapName = getMapName(pAiMaterial, aiTextureType_DIFFUSE_ROUGHNESS);
		mat.aoMapName = getMapName(pAiMaterial, aiTextureType_AMBIENT_OCCLUSION);
		materails.push_back(std::move(mat));
		return materialIndex;
	} 
	return -1;
}

void AssimpLoader::processBone(std::vector<SkinnedVertex> &vertices,
	std::vector<float4x4> &boneOffsets,
	aiMesh *pAiMesh)
{
	std::vector<std::int8_t> vertexCurrBoneCount;
	vertexCurrBoneCount.resize(vertices.size(), 0);
	auto fillVertexWeights = [&](size_t boneIdx, aiBone *pAiBone) {
		for (size_t j = 0; j < pAiBone->mNumWeights; ++j) {
			size_t vertexId = pAiBone->mWeights[j].mVertexId;
			size_t boneId = vertexCurrBoneCount[vertexId];
			float weight = pAiBone->mWeights[j].mWeight;
			if (boneId < 4) {
				vertices[vertexId].boneIndices[boneId] = boneIdx;
				if (boneId < 3) 
					vertices[vertexId].boneWeights[boneId] = weight;
			}
			++vertexCurrBoneCount[vertexId];
		}
	};

	std::unordered_map<std::string, size_t> boneName2Index;
	for (size_t i = 0; i < pAiMesh->mNumBones; ++i) {
		aiBone *pAiBone = pAiMesh->mBones[i];
		std::string boneName{ pAiBone->mName.data, pAiBone->mName.length };
		size_t boneIdx;
		if (auto iter = boneName2Index.find(boneName); iter != boneName2Index.end()) {
			boneIdx = iter->second;
		} else {
			boneIdx = boneOffsets.size();
			boneOffsets.push_back(float4x4::identity());
			boneName2Index[boneName] = boneIdx;
		}

		boneOffsets[boneIdx] = convertFloat4x4(pAiBone->mOffsetMatrix);
		fillVertexWeights(boneIdx, pAiBone);
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

}
