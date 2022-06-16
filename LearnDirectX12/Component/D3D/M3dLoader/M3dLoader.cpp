#include <D3D/M3dLoader/M3dLoader.h>
#include <fstream>

namespace d3d {

bool M3dLoader::loadM3d(const std::string &fileName, 
	std::vector<com::Vertex> &vertices,
	std::vector<uint16_t> &indices, 
	std::vector<d3d::SubMesh> &subMesh, 
	std::vector<M3dMaterial> &materials)
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

	readMaterials(fin, numMaterials, materials);
	readSubsetTable(fin, numMaterials, subMesh);
	readVertex(fin, numVertices, vertices);
	readTriangles(fin, numTriangles, indices);
	return true;
}

bool M3dLoader::loadM3d(const std::string &fileName, 
	std::vector<SkinnedVertex> &vertices,
	std::vector<uint16_t> &indices, 
	std::vector<d3d::SubMesh> &subMesh, 
	std::vector<M3dMaterial> &materials,
	SkinnedData &skinInfo)
{
	std::ifstream fin(fileName);
	if (!fin.is_open())
		return false;

	UINT numMaterials = 0;
	UINT numVertices = 0;
	UINT numTriangles = 0;
	UINT numBones = 0;
	UINT numAnimationClips = 0;

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
	float4 albedo;
	float3 fresnelR0;
	for (size_t i = 0; i < numMaterials; ++i) {
		fin >> ignore >> mat[i].name;
		fin >> ignore >> albedo.x >> albedo.y >> albedo.z >> albedo.w;
		fin >> ignore >> fresnelR0.x >> fresnelR0.y >> fresnelR0.z;
		fin >> ignore >> mat[i].roughness;
		fin >> ignore >> mat[i].alphaClip;
		fin >> ignore >> mat[i].materialTypeName;
		fin >> ignore >> mat[i].diffuseMapName;
		fin >> ignore >> mat[i].normalMapName;
		mat[i].diffuseAlbedo = albedo;
		mat[i].fresnelR0 = fresnelR0;
	}
}

}
