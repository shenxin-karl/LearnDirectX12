#pragma once
#include "D3D/Model/MeshModel/MeshModel.h"

namespace d3d {

using namespace Math;

class ALMaterial;
class ALTree;
struct ALMesh : public rgph::IMesh {
	using BoneIndex = std::array<uint8_t, 4>;
	ALMesh(ALTree *pTree, std::string_view modelPath, size_t nodeIdx, size_t meshIdx, const aiMesh *pAiMesh);
	const ALMaterial *getMaterial() const;
	size_t getMeshIdx() const;
	const std::string &getMeshName() const override;
	const std::vector<float4> &getPositions() const override;
	const std::vector<float3> &getNormals() const override;
	const std::vector<float3> &getTangents() const override;
	const std::vector<float2> &getTexcoord0() const override;
	const std::vector<float2> &getTexcoord1() const override;
	const std::vector<BoneIndex> &getBoneIndices() const override;
	const std::vector<float3> &getBoneWeight() const override;
	const std::vector<uint32_t> &getIndices() const override;
	const AxisAlignedBox &getBoundingBox() const override;
	bool saveToObj(const std::string &fileName) const;
private:
	using BoneIndex = std::array<uint8_t, 4>;
	const ALMaterial *_pMaterial;
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
}