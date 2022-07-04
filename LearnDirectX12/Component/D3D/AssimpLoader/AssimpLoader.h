#pragma once
#include <optional>
#include <Geometry/GeometryGenerator.h>
#include <D3D/d3dutil.h>
#include <D3D/Animation/SkinnedData.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <dx12lib/Context/ContextProxy.hpp>

namespace d3d {

class AssimpLoader {
public:
	struct ALMesh {
		std::vector<com::Vertex> vertices;
		std::vector<uint16_t> indices;
		aiMaterial *pAiMaterial = nullptr;
	};
	struct ALSkinnedMesh {
		std::vector<SkinnedVertex> vertices;
		std::vector<uint16_t> indices;
		aiMaterial *pAiMaterial = nullptr;
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
	void prepareTexture(dx12lib::CommonContextProxy pCommonCtx) const;
	std::vector<ALMesh> parseMesh() const;
	std::vector<ALSkinnedMesh> parseSkinnedMesh() const;
	const std::string &getFileName() const;
	const aiScene *getScene() const;
	size_t getTextureCount() const;
	std::string getTextureName(size_t i) const;
	std::optional<std::string> getTextureName(const aiMaterial *pAiMaterial, aiTextureType type, size_t index = 0) const;
	static float4x4 convertFloat4x4(const aiMatrix4x4 &m);
	static float3 convertFloat3(const aiVector3D &v);
	static float4 convertFloat4(const aiQuaternion &q);
private:
	static void processTriangles(std::vector<uint16_t> &indices, const aiMesh *pAiMesh);
	static void processVertices(std::vector<com::Vertex> &vertices, const aiMesh *pAiMesh, Matrix4 toLocalModel);
	static void processSkinnedVertices(std::vector<SkinnedVertex> &vertices, const aiMesh *pAiMesh);
	void parseMeshImpl(std::vector<AssimpLoader::ALMesh> &meshs, const aiNode *pAiNode, Matrix4 toParentSpace) const;
	void processBoneOffsets(std::vector<SkinnedVertex> &vertices, BoneInfo &boneInfo, const aiMesh *pAiMesh) const;
	void processBoneHierarchy() const;
	void processBoneHierarchyAndAnimation(std::vector<ALSkinnedMesh> &meshs, const std::vector<BoneInfo> &boneInfos) const;
private:
	bool _isLoad = false;
	const aiScene *_pScene = nullptr;
	std::string _fileName;
	Assimp::Importer _importer;

	mutable std::vector<size_t> _boneHierarchy;
	mutable std::vector<std::string> _boneNames;
	mutable std::unordered_map<std::string, AiNodeInfo> _boneInfoMap;
};



}
