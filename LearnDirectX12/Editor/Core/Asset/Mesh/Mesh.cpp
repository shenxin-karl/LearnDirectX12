#include "Mesh.h"

namespace core {

Mesh::Mesh(const std::string &importPath, const std::string &savePath) {
}

Mesh::Mesh(const MeshMetaDesc &desc) {
}

const std::string & Mesh::getAssetName() const {
	return _meshName;
}

const std::string & Mesh::getAssetPath() const {
	return _meshPath;
}

void Mesh::saveMeta() const {

}

const std::string & Mesh::getType() const {
	return _type;
}

size_t Mesh::getVertexCount() const {
	return _vertexCount;
}

size_t Mesh::getIndexCount() const {
	return _indexCount;
}

bool Mesh::hasNormal() const {
	return _hasNormal;
}

bool Mesh::hasUV0() const {
	return _hasUV0;
}

bool Mesh::hasUV1() const {
	return _hasUV1;
}

bool Mesh::hasTangent() const {
	return _hasTangent;
}

bool Mesh::hasBoneWeight() const {
	return _hasBoneWeight;
}

}
