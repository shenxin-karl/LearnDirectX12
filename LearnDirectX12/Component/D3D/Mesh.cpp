#include "Mesh.h"


namespace d3d {

UnionMesh::UnionMesh(std::shared_ptr<dx12lib::VertexBuffer> pVertexBuffer, 
	std::shared_ptr<dx12lib::IndexBuffer> pIndexBuffer, 
	const std::unordered_map<std::string, SubMesh> &subMeshs) 
: _subMeshs(subMeshs)
{
	_pIndexBuffer = pIndexBuffer;
	_pVertexBuffer = pVertexBuffer;
}

bool UnionMesh::hasSubMesh(const std::string &name, SubMesh &subMesh) const {
	bool ret = false;
	if (auto iter = _subMeshs.find(name); iter != _subMeshs.end()) {
		subMesh = iter->second;
		ret = true;
	}
	return ret;
}

}
