#include "ALMesh.h"
#include <fstream>
#include <format>

namespace d3d {

ALMesh::ALMesh(ALTree *pTree, std::string_view modelPath, size_t nodeIdx, size_t meshIdx, const aiMesh *pAiMesh)
: _pMaterial(pTree->getMaterial(pAiMesh->mMaterialIndex)), _meshIdx(meshIdx)
, _meshName(std::format("{}_{}_{}", modelPath, nodeIdx, meshIdx))
{
	_positions.reserve(pAiMesh->mNumVertices);
	if (pAiMesh->HasNormals())
		_normals.reserve(pAiMesh->mNumVertices);
	if (pAiMesh->HasTangentsAndBitangents())
		_tangents.reserve(pAiMesh->mNumVertices);
	if (pAiMesh->HasTextureCoords(0))
		_texcoord0.reserve(pAiMesh->mNumVertices);
	if (pAiMesh->HasTextureCoords(1))
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

const std::string &ALMesh::getMeshName() const {
	return _meshName;
}

const std::vector<float4> &ALMesh::getPositions() const {
	return _positions;
}

const std::vector<float3> &ALMesh::getNormals() const {
	return _normals;
}

const std::vector<float3> &ALMesh::getTangents() const {
	return _tangents;
}

const std::vector<float2> &ALMesh::getTexcoord0() const {
	return _texcoord0;
}

const std::vector<float2> &ALMesh::getTexcoord1() const {
	return _texcoord1;
}

const std::vector<ALMesh::BoneIndex> &ALMesh::getBoneIndices() const {
	return _boneIndices;
}

const std::vector<float3> &ALMesh::getBoneWeight() const {
	return _boneWeight;
}

const std::vector<uint32_t> &ALMesh::getIndices() const {
	return _indices;
}

const AxisAlignedBox &ALMesh::getBoundingBox() const {
	return _boundingBox;
}

bool ALMesh::saveToObj(const std::string &fileName) const {
	const auto &indices = getIndices();
	const auto &positions = getPositions();
	const auto &normals = getNormals();
	const auto &texcoord0 = getTexcoord0();
	if (positions.size() < 2)
		return false;

	std::fstream fout(fileName, std::ios::out);
	if (!fout.is_open())
		return false;

	std::stringstream posBuf;
	std::stringstream texBuf;
	std::stringstream norBuf;
	std::stringstream idxBuf;

	for (size_t i = 0; i < positions.size(); ++i) {
		const float4 &position = positions[i];
		posBuf << "v " << position.x
			   << " " << position.y
			   << " " << position.z
			   << '\n';
		if (!normals.empty()) {
			const float3 &normal = normals[i];
			norBuf << "vn " << normal.x
				   << " " << normal.y
				   << " " << normal.z
				   << '\n';
		}

		if (!texcoord0.empty()) {
			const float2 &tex0 = texcoord0[i];
			texBuf << "vt " << tex0.x
				   << " " << tex0.y
				   << " " << 0.0f
				   << '\n';
		}
	}

	auto writeIndex = [&](size_t idx0, size_t idx1, size_t idx2) {
		if (!normals.empty()) {
			if (!texcoord0.empty()) {
				idxBuf << "f " << idx0 << "/" << idx0 << "/" << idx0
					   << " " << idx1 << "/" << idx1 << "/" << idx1
					   << " " << idx2 << "/" << idx2 << "/" << idx2
					   << "\n";
			} else {
				idxBuf << "f " << idx0 << "/" << idx0
					   << " " << idx1 << "/" << idx1
					   << " " << idx2 << "/" << idx2
					   << "\n";
			}
		}
	};

	if  (!indices.empty()) {
		for (size_t i = 0; i < indices.size() - 2; i += 3) {
			size_t idx0 = indices[i + 0] + 1;
			size_t idx1 = indices[i + 1] + 1;
			size_t idx2 = indices[i + 2] + 1;
			writeIndex(idx0, idx1, idx2);
		}
	} else {
		for (size_t i = 0; i < positions.size()-2; i += 3)
			writeIndex(i+1, i+2, i+3);
	}

	fout << posBuf.rdbuf() << std::endl;
	fout << texBuf.rdbuf() << std::endl;
	fout << norBuf.rdbuf() << std::endl;
	fout << idxBuf.rdbuf() << std::endl;
	fout.close();
	return true;
}
	
}