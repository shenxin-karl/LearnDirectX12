#pragma once
#include <Geometry/GeometryGenerator.h>
#include <D3D/d3dutil.h>
#include <D3D/Animation/SkinnedData.h>
#include <optional>

namespace d3d {

class AssimpLoader {
	struct ALMesh {
		std::vector<com::Vertex> vertices;
		std::vector<uint16_t> indices;
		size_t materialIndex = -1;
	};
	struct ALSkinnedMesh {
		std::vector<SkinnedVertex> vertices;
		std::vector<uint16_t> indices;
		size_t materialIndex = -1;
		SkinnedData skinnedData;
	};
	struct BoneInfo {
		std::vector<float4x4> boneOffsets;
		std::vector<std::string> boneNames;
		std::unordered_map<std::string, uint8_t> boneIndexMap;
	};
	struct AiNodeInfo {
		size_t index;
		float4x4 nodeTransform;
	};
public:
	explicit AssimpLoader(const std::string &fileName, bool bLoad = false);
	bool load();
	bool isLoad() const;
	std::vector<ALMesh> parseMesh() const;
	std::vector<ALSkinnedMesh> parseSkinnedMesh() const;
	static float4x4 convertFloat4x4(const aiMatrix4x4 &m);
	static float3 convertFloat3(const aiVector3D &v);
	static float4 convertFloat4(const aiQuaternion &q);
private:
	static void processTriangles(std::vector<uint16_t> &indices, const aiMesh *pAiMesh);
	static void processVertices(std::vector<com::Vertex> &vertices, const aiMesh *pAiMesh);
	static void processSkinnedVertices(std::vector<SkinnedVertex> &vertices, const aiMesh *pAiMesh);
	static void processBoneOffsets(BoneInfo &boneInfo, const aiMesh *pAiMesh);
	void processBoneHierarchyAndAnimation(std::vector<ALSkinnedMesh> &meshs, const std::vector<BoneInfo> &boneInfos) const;
private:
	bool _isLoad = false;
	const aiScene *_pScene = nullptr;
	std::string _fileName;
	Assimp::Importer _importer;
};



}
