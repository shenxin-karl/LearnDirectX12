#pragma once
#include <Geometry/GeometryGenerator.h>
#include <D3D/d3dutil.h>
#include <D3D/Animation/SkinnedData.h>
#include <optional>

namespace d3d {

class AssimpLoader {
	struct ALMaterial {
		std::string albedoMapName;			// SRGB 
		std::string normalMapName;			
		std::string metallicMapName;
		std::string roughnessMapName;
		std::string aoMapName;				// SRGB
	};

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

	static std::optional<std::string> load(const std::string &fileName,
		std::vector<ALMesh> &meshes,
		std::vector<ALMaterial> &materials
	);

	static std::optional<std::string> load(const std::string &fileName,
		std::vector<ALSkinnedMesh> meshes,
		std::vector<ALMaterial> &materails
	);
private:
	static void processNode(std::vector<ALMesh> &meshes, std::vector<ALMaterial> &materails, aiNode *pAiNode, const aiScene *pScene);
	static void processNode(std::vector<ALSkinnedMesh> &meshes, std::vector<ALMaterial> &materails, aiNode *pAiNode, const aiScene *pScene);
	static void processMesh(std::vector<ALMesh> &meshes, std::vector<ALMaterial> &materials, aiMesh *pAiMesh, const aiScene *pScene);
	static void processMesh(std::vector<ALSkinnedMesh> &meshes, std::vector<ALMaterial> &materails, aiMesh *pAiMesh, const aiScene *pScene);
	static void processTriangles(std::vector<uint16_t> &indices, aiMesh *pAiMesh);
	static size_t processMaterials(std::vector<ALMaterial> &materails, aiMesh *pAiMesh, const aiScene *pScene);
	static void processBone(std::vector<SkinnedVertex> &vertices, std::vector<float4x4> &boneOffsets, aiMesh *pAiMesh);
	static float4x4 convertFloat4x4(const aiMatrix4x4 &m);
};



}
