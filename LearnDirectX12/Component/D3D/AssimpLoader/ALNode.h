#pragma once
#include <string>
#include <array>
#include <vector>
#include <Math/MathStd.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace d3d {

using namespace Math;

class ALMaterial;
class ALTree;
struct ALMesh {
	using BoneIndex = std::array<uint8_t, 4>;
	ALMesh(ALTree *pTree, std::string_view modelPath, size_t nodeIdx, size_t meshIdx, const aiMesh *pAiMesh);
	const ALMaterial *getMaterial() const;
	size_t getMeshIdx() const;
	const std::string &getMeshName() const;
	const std::vector<float4>    &getPositions() const;
	const std::vector<float3>    &getNormals() const;
	const std::vector<float3>    &getTangents() const;
	const std::vector<float2>    &getTexcoord0() const;
	const std::vector<float2>    &getTexcoord1() const;
	const std::vector<BoneIndex> &getBoneIndices() const;
	const std::vector<float3>    &getBoneWeight() const;
	const std::vector<uint32_t>  &getIndices() const;
	const AxisAlignedBox		 &getBoundingBox() const;
private:
	using BoneIndex = std::array<uint8_t, 4>;
	const ALMaterial	  *_pMaterial;
	size_t				   _meshIdx;
	AxisAlignedBox		   _boundingBox;
	std::string			   _meshName;
	std::vector<float4>    _positions;
	std::vector<float3>    _normals;
	std::vector<float3>    _tangents;
	std::vector<float2>    _texcoord0;
	std::vector<float2>    _texcoord1;
	std::vector<BoneIndex> _boneIndices;
	std::vector<float3>	   _boneWeight;
	std::vector<uint32_t>  _indices;
};

class ALNode {
public:
	ALNode(ALTree *pTree, std::string_view modelPath, int id, const aiScene *pAiScene, const aiNode *pAiNode);
	ALNode(const ALNode &) = delete;
	int getNodeId() const;
	size_t getNumChildren() const;
	const ALNode *getChildren(size_t idx) const;
	const float4x4 &getNodeTransform() const;
	size_t getNumMesh() const;
	std::shared_ptr<ALMesh> getMesh(size_t idx) const;
private:
	int _nodeId;
	unsigned int _numChildren;
	float4x4 _nodeTransform;
	std::vector<std::shared_ptr<ALMesh>> _meshs;
	std::vector<std::unique_ptr<ALNode>> _children;
};

}
