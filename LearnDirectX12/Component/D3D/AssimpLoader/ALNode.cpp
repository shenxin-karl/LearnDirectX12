#include "ALNode.h"
#include <format>
#include <DirectXMath.h>
#include "ALTree.h"


namespace d3d {

ALMesh::ALMesh(ALTree *pTree, std::string_view modelPath, size_t nodeIdx, size_t meshIdx, const aiMesh *pAiMesh)
: _pMaterial(pTree->getMaterial(pAiMesh->mMaterialIndex)), _meshIdx(meshIdx)
, _meshName(std::format("{}_{}_{}", modelPath, nodeIdx, meshIdx))
{
	_positions.reserve(pAiMesh->mNumVertices);
	if (pAiMesh->mNormals)
		_normals.reserve(pAiMesh->mNumVertices);
	if (pAiMesh->mTangents)
		_tangents.reserve(pAiMesh->mNumVertices);
	if (pAiMesh->mTextureCoords[0])
		_texcoord0.reserve(pAiMesh->mNumVertices);
	if (pAiMesh->mTextureCoords[1])
		_texcoord1.reserve(pAiMesh->mNumVertices);

	for (unsigned i = 0; i < pAiMesh->mNumVertices; ++i) {
		const aiVector3D &pos = pAiMesh->mVertices[i];
		_positions.emplace_back(pos.x, pos.y, pos.z, 1.0);
		if (pAiMesh->mNormals) {
			const aiVector3D &nrm = pAiMesh->mNormals[i];
			_normals.emplace_back(nrm.x, nrm.y, nrm.z);
		}
		if (pAiMesh->mTangents) {
			const aiVector3D &tan = pAiMesh->mTangents[i];
			_tangents.emplace_back(tan.x, tan.y, tan.z);
		}
		if (pAiMesh->mTextureCoords[0]) {
			const aiVector3D &tex0 = pAiMesh->mTextureCoords[0][i];
			_texcoord0.emplace_back(tex0.x, tex0.y);
		}
		if (pAiMesh->mTextureCoords[1]) {
			const aiVector3D &tex1 = pAiMesh->mTextureCoords[1][i];
			_texcoord1.emplace_back(tex1.x, tex1.y);
		}
	}

	for (unsigned i = 0; i < pAiMesh->mNumFaces; ++i) {
		for (size_t j = 0; j < pAiMesh->mFaces[i].mNumIndices; ++j)
			_indices.push_back(pAiMesh->mFaces[i].mIndices[j]);
	}

	const auto &aabb = pAiMesh->mAABB;
	_boundingBox = AxisAlignedBox(
		Vector3(aabb.mMin.x, aabb.mMin.y, aabb.mMin.z),
		Vector3(aabb.mMax.x, aabb.mMax.y, aabb.mMax.z)
	);
}

const ALMaterial *ALMesh::getMaterial() const {
	return _pMaterial;
}

size_t ALMesh::getMeshIdx() const {
	return _meshIdx;
}

const std::string & ALMesh::getMeshName() const {
	return _meshName;
}

const std::vector<float4> & ALMesh::getPositions() const {
	return _positions;
}

const std::vector<float3> & ALMesh::getNormals() const {
	return _normals;
}

const std::vector<float3> & ALMesh::getTangents() const {
	return _tangents;
}

const std::vector<float2> & ALMesh::getTexcoord0() const {
	return _texcoord0;
}

const std::vector<float2> & ALMesh::getTexcoord1() const {
	return _texcoord1;
}

const std::vector<ALMesh::BoneIndex> & ALMesh::getBoneIndices() const {
	return _boneIndices;
}

const std::vector<float3> & ALMesh::getBoneWeight() const {
	return _boneWeight;
}

const std::vector<uint32_t> & ALMesh::getIndices() const {
	return _indices;
}

const AxisAlignedBox & ALMesh::getBoundingBox() const {
	return _boundingBox;
}

ALNode::ALNode(ALTree *pTree, std::string_view modelPath, int id, const aiScene *pAiScene, const aiNode *pAiNode)
: _nodeId(id), _numChildren(pAiNode->mNumChildren) {
	for (size_t i = 0; i < pAiNode->mNumMeshes; ++i) {
		unsigned int meshIdx = pAiNode->mMeshes[i];
		_meshs.push_back(std::make_shared<ALMesh>(pTree, modelPath, _nodeId, meshIdx, pAiScene->mMeshes[meshIdx]));
	}

	aiVector3D scale;
	aiVector3D position;
	aiQuaternion rotate;
	pAiNode->mTransformation.Decompose(scale, rotate, position);

	_nodeTransform = float4x4(Matrix4(DirectX::XMMatrixAffineTransformation(
		Vector3(scale.x, scale.y, scale.z),
		Vector3(0.f),
		Quaternion(rotate.x, rotate.y, rotate.z, rotate.w),
		Vector3(position.x, position.y, position.z)
	)));

	for (size_t i = 0; i < pAiNode->mNumChildren; ++i) {
		++id;
		_children.push_back(std::make_unique<ALNode>(
			pTree,
			modelPath,
			id,
			pAiScene,
			pAiNode->mChildren[i]
		));
	}
}

int ALNode::getNodeId() const {
	return _nodeId;
}

size_t ALNode::getNumChildren() const {
	return _numChildren;
}

const ALNode * ALNode::getChildren(size_t idx) const {
	return _children[idx].get();
}

const float4x4 & ALNode::getNodeTransform() const {
	return _nodeTransform;
}

size_t ALNode::getNumMesh() const {
	return _meshs.size();
}

std::shared_ptr<ALMesh> ALNode::getMesh(size_t idx) const {
	if (idx >= getNumMesh()) {
		assert(false);
		return nullptr;
	}
	return _meshs[idx];
}
}
