#include <D3D/M3dLoader/M3dLoader.h>
#include <fstream>
#include <intsafe.h>

namespace d3d {

bool M3dLoader::loadM3d(const std::string &fileName, 
	std::vector<com::Vertex> &vertices,
	std::vector<uint16_t> &indices, 
	std::vector<Subset> &subMesh,
	std::vector<M3dMaterial> &materials) {
	std::ifstream fin(fileName);
	if (!fin.is_open())
		return false;

	size_t numMaterials = 0;
	size_t numVertices = 0;
	size_t numTriangles = 0;
	size_t numBones = 0;
	size_t numAnimationClips = 0;

	std::string ignore;
	fin >> ignore; // file header text
	fin >> ignore >> numMaterials;
	fin >> ignore >> numVertices;
	fin >> ignore >> numTriangles;
	fin >> ignore >> numBones;
	fin >> ignore >> numAnimationClips;

	readMaterials(fin, numMaterials, materials);
	readSubsetTable(fin, numMaterials, subMesh);
	readVertex(fin, numVertices, vertices);
	readTriangles(fin, numTriangles, indices);
	return true;
}

bool M3dLoader::loadM3d(const std::string &fileName,
	std::vector<SkinnedVertex> &vertices,
	std::vector<uint16_t> &indices,
	std::vector<Subset> &subMesh,
	std::vector<M3dMaterial> &materials,
	SkinnedData &skinInfo)
{
	std::ifstream fin(fileName);
	if (!fin.is_open())
		return false;

	size_t numMaterials = 0;
	size_t numVertices = 0;
	size_t numTriangles = 0;
	size_t numBones = 0;
	size_t numAnimationClips = 0;

	std::string ignore;
	fin >> ignore; // file header text
	fin >> ignore >> numMaterials;
	fin >> ignore >> numVertices;
	fin >> ignore >> numTriangles;
	fin >> ignore >> numBones;
	fin >> ignore >> numAnimationClips;

	std::vector<float4x4> boneOffsets;
	std::vector<size_t> boneIndexToParentIndex;
	std::unordered_map<std::string, AnimationClip> animations;

	readMaterials(fin, numMaterials, materials);
	readSubsetTable(fin, numMaterials, subMesh);
	readSkinnedVertices(fin, numVertices, vertices);
	readTriangles(fin, numTriangles, indices);
	readBoneOffsets(fin, numBones, boneOffsets);
	readBoneHierarchy(fin, numBones, boneIndexToParentIndex);
	readAnimationClips(fin, numBones, numAnimationClips, animations);

	skinInfo.setBoneOffsets(std::move(boneOffsets));
	skinInfo.setBoneHierarchy(std::move(boneIndexToParentIndex));
	skinInfo.setAnimations(std::move(animations));
	return true;
}

void M3dLoader::readMaterials(std::ifstream &fin, size_t numMaterials, std::vector<M3dMaterial> &mat) {
	std::string ignore;
	mat.resize(numMaterials);

	fin >> ignore;
	float3 albedo;
	float3 fresnelR0;
	for (size_t i = 0; i < numMaterials; ++i) {
		fin >> ignore >> mat[i].name;
		fin >> ignore >> albedo.x >> albedo.y >> albedo.z;
		fin >> ignore >> fresnelR0.x >> fresnelR0.y >> fresnelR0.z;
		fin >> ignore >> mat[i].roughness;
		fin >> ignore >> mat[i].alphaClip;
		fin >> ignore >> mat[i].materialTypeName;
		fin >> ignore >> mat[i].diffuseMapName;
		fin >> ignore >> mat[i].normalMapName;
		mat[i].diffuseAlbedo = float4(albedo, 1.f);
		mat[i].fresnelR0 = fresnelR0;
	}
}

void M3dLoader::readSubsetTable(std::ifstream &fin, size_t numSubsets, std::vector<Subset> &subsets) {
	std::string ignore;
	subsets.resize(numSubsets);

	fin >> ignore; // file header text
	for (size_t i = 0; i < numSubsets; ++i) {
		fin >> ignore >> subsets[i].id;
		fin >> ignore >> subsets[i].vertexStart;
		fin >> ignore >> subsets[i].vertexCount;
		fin >> ignore >> subsets[i].faceStart;
		fin >> ignore >> subsets[i].faceCount;
	}
}

void M3dLoader::readVertex(std::ifstream &fin, size_t numVertices, std::vector<com::Vertex> &vertices) {
	std::string ignore;
	vertices.resize(numVertices);

	float det;
	fin >> ignore; // vertices header text
	for (size_t i = 0; i < numVertices; ++i) {
		auto &v = vertices[i];
		fin >> ignore >> v.position.x >> v.position.y >> v.position.z;
		fin >> ignore >> v.tangent.x  >> v.tangent.y  >> v.tangent.z   >> det;
		fin >> ignore >> v.normal.x   >> v.normal.y   >> v.normal.z;
		fin >> ignore >> v.texcoord.x >> v.texcoord.y;
	}
}

void M3dLoader::readSkinnedVertices(std::ifstream &fin, size_t numVertices, std::vector<SkinnedVertex> &vertices) {
	std::string ignore;
	vertices.resize(numVertices);

	float det;
	float weights[4] = { 0.f };
	size_t boneIndices[4] = { 0 };
	fin >> ignore; // vertices header text
	for (size_t i = 0; i < numVertices; ++i) {
		auto &v = vertices[i];
		fin >> ignore >> v.position.x   >> v.position.y   >> v.position.z;
		fin >> ignore >> v.tangent.x    >> v.tangent.y    >> v.tangent.z    >> det;
		fin >> ignore >> v.normal.x     >> v.normal.y     >> v.normal.z;
		fin >> ignore >> v.texcoord.x   >> v.texcoord.y;
		fin >> ignore >> weights[0]     >> weights[1]     >> weights[2]     >> weights[3];
		fin >> ignore >> boneIndices[0] >> boneIndices[1] >> boneIndices[2] >> boneIndices[3];

		v.boneWeights.x = weights[0];
		v.boneWeights.y = weights[1];
		v.boneWeights.z = weights[2];

		v.boneIndices[0] = static_cast<uint8_t>(boneIndices[0]);
		v.boneIndices[1] = static_cast<uint8_t>(boneIndices[1]);
		v.boneIndices[2] = static_cast<uint8_t>(boneIndices[2]);
		v.boneIndices[3] = static_cast<uint8_t>(boneIndices[3]);
	}
}

void M3dLoader::readTriangles(std::ifstream &fin, size_t numTriangles, std::vector<uint16_t> &indices) {
	std::string ignore;
	assert(numTriangles > 0);
	indices.resize(numTriangles * 3);

	fin >> ignore; // triangles header text
	for (size_t i = 0; i < numTriangles; ++i) {
		fin >> indices[i*3 + 0];
		fin >> indices[i*3 + 1];
		fin >> indices[i*3 + 2];
	}
}

void M3dLoader::readBoneOffsets(std::ifstream &fin, size_t numBones, std::vector<float4x4> &boneOffsets) {
	std::string ignore;
	boneOffsets.resize(numBones);

	fin >> ignore; // BoneOffsets header text
	for (size_t i = 0; i < numBones; ++i) {
		auto &m = boneOffsets[i];
		fin >> ignore;
		fin >> m(0, 0) >> m(0, 1) >> m(0, 2) >> m(0, 3);
		fin >> m(1, 0) >> m(1, 1) >> m(1, 2) >> m(1, 3);
		fin >> m(2, 0) >> m(2, 1) >> m(2, 2) >> m(2, 3);
		fin >> m(3, 0) >> m(3, 1) >> m(3, 2) >> m(3, 3);
	}
}

void M3dLoader::readBoneHierarchy(std::ifstream &fin, size_t numBones, std::vector<size_t> &boneIndexToParentIndex) {
	std::string ignore;
	boneIndexToParentIndex.resize(numBones);

	fin >> ignore; // BoneHierarchy header text
	for (size_t i = 0; i < numBones; ++i) {
		fin >> ignore;
		fin >> boneIndexToParentIndex[i];
	}
}

void M3dLoader::readAnimationClips(std::ifstream &fin, 
	size_t numBones, 
	size_t numAnimationClips,
	std::unordered_map<std::string, AnimationClip> &animations)
{
	std::string ignore;
	std::string clipName;

	fin >> ignore;	// AnimationClips header text
	for (size_t clipIndex = 0; clipIndex  < numAnimationClips; ++clipIndex) {
		fin >> ignore >> clipName;
		fin >> ignore;				// {

		AnimationClip animationClip;
		animationClip.boneAnimations.resize(numBones);
		for (size_t boneIdx = 0; boneIdx < numBones; ++boneIdx) 
			readBoneKeyframes(fin, animationClip.boneAnimations[boneIdx]);

		fin >> ignore;				// }
		animations[clipName] = std::move(animationClip);
	}
}

void M3dLoader::readBoneKeyframes(std::ifstream &fin, BoneAnimation &boneAnimation) {
	std::string ignore;
	size_t keyframes = 0;
	fin >> ignore >> ignore >> keyframes;
	fin >> ignore;	// {

	boneAnimation.keyframes.resize(keyframes);
	for (size_t i = 0; i < keyframes; ++i) {
		auto &v = boneAnimation.keyframes[i];
		auto &timePoint = v.timePoint;
		auto &t = v.translation;
		auto &s = v.scale;
		auto &q = v.rotationQuat;

		fin >> ignore >> timePoint;
		fin >> ignore >> t.x >> t.y >> t.z;
		fin >> ignore >> s.x >> s.y >> s.z;
		fin >> ignore >> q.x >> q.y >> q.z >> q.w;
	}

	fin >> ignore;	// }
}

}
