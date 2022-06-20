#pragma once
#include <Geometry/GeometryGenerator.h>
#include <D3D/Animation/SkinnedData.h>

namespace d3d {

class M3dLoader {
public:
	struct M3dMaterial {
		std::string name;
		float4		diffuseAlbedo = float4(0.1f);
		float3		fresnelR0     = float3(0.01f);
		float		roughness     = 0.8f;
		bool		alphaClip     = false;
		std::string materialTypeName;
		std::string diffuseMapName;
		std::string normalMapName;
	};

	struct Subset {
		size_t  id			= -1;
		size_t  vertexStart = 0;		// 没有索引时才能使用
		size_t  vertexCount = 0;
		size_t  faceStart	= 0;		// 转换为索引起始地址时需要乘3
		size_t  faceCount	= 0;		// 转换为索引数是需要乘3
	};

	static bool loadM3d(const std::string &fileName, 
		std::vector<com::Vertex> &vertices,
		std::vector<uint16_t> &indices,
		std::vector<Subset> &subMesh,
		std::vector<M3dMaterial> &materials
	);

	static bool loadM3d(const std::string &fileName, 
		std::vector<SkinnedVertex> &vertices,
		std::vector<uint16_t> &indices,
		std::vector<Subset> &subMesh,
		std::vector<M3dMaterial> &materials,
		SkinnedData &skinInfo
	);
private:
	static void readMaterials(std::ifstream &fin, size_t numMaterials, std::vector<M3dMaterial> &mat);
	static void readSubsetTable(std::ifstream &fin, size_t numSubsets, std::vector<Subset> &subsets);
	static void readVertex(std::ifstream &fin, size_t numVertices, std::vector<com::Vertex> &vertices);
	static void readSkinnedVertices(std::ifstream &fin, size_t numVertices, std::vector<SkinnedVertex> &vertices);
	static void readTriangles(std::ifstream &fin, size_t numTriangles, std::vector<uint16_t> &indices);
	static void readBoneOffsets(std::ifstream &fin, size_t numBones, std::vector<float4x4> &boneOffsets);
	static void readBoneHierarchy(std::ifstream &fin, size_t numBones, std::vector<size_t> &boneIndexToParentIndex);
	static void readAnimationClips(std::ifstream &fin, size_t numBones, size_t numAnimationClips, std::unordered_map<std::string, AnimationClip> &animations);
	static void readBoneKeyframes(std::ifstream &fin, BoneAnimation &boneAnimation);
};

}
