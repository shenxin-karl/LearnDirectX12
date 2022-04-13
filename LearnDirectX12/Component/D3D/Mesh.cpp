#include "Mesh.h"
#include "dx12lib/VertexBuffer.h"
#include "dx12lib/IndexBuffer.h"

namespace d3d {

SubMesh::operator bool() const {
	return _count != -1;
}

void SubMesh::drawInstanced(dx12lib::GraphicsContextProxy pGrahpicsCtx,
	size_t instanceCount /*= 1*/,
	size_t startInstanceLocation /*= 0 */) const {
	assert(bool(*this) && "this submesh invalid");
	pGrahpicsCtx->drawInstanced(
		_count,
		instanceCount,
		_baseVertexLocation,
		startInstanceLocation
	);
}

void SubMesh::drawIndexdInstanced(dx12lib::GraphicsContextProxy pGrahpicsCtx,
	size_t instanceCount /*= 1*/,
	size_t startInstanceLocation /*= 0 */) const
{
	assert(bool(*this) && "this submesh invalid");
	pGrahpicsCtx->drawIndexedInstanced(
		_count,
		instanceCount,
		_startIndexLocation,
		_baseVertexLocation,
		startInstanceLocation
	);
}

Mesh::Mesh(std::shared_ptr<dx12lib::VertexBuffer> pVertexBuffer, 
	std::shared_ptr<dx12lib::IndexBuffer> pIndexBuffer,
	const DX::BoundingBox &bounds,
	const std::vector<SubMesh> &subMeshs) 
: _pVertexBuffer(pVertexBuffer), _pIndexBuffer(pIndexBuffer), _subMeshs(subMeshs), _bounds(bounds)
{
}

void Mesh::appendSubMesh(const SubMesh &submesh) {
	_subMeshs.emplace_back(submesh);
}

std::shared_ptr<dx12lib::VertexBuffer> Mesh::getVertexBuffer() const {
	return _pVertexBuffer;
}

std::shared_ptr<dx12lib::IndexBuffer> Mesh::getIndexBuffer() const {
	return _pIndexBuffer;
}

Mesh::iteraotr Mesh::begin() const {
	return _subMeshs.begin();
}

Mesh::iteraotr Mesh::end() const {
	return _subMeshs.end();
}


void Mesh::drawInstanced(dx12lib::GraphicsContextProxy pGrahpicsCtx,
	size_t instanceCount /*= 1*/,
	size_t startInstanceLocation /*= 0 */) const {
	if (!_subMeshs.empty()) {
		for (auto &submesh : *this)
			submesh.drawInstanced(pGrahpicsCtx, instanceCount, startInstanceLocation);
	} else {
		pGrahpicsCtx->drawInstanced(
			_pVertexBuffer->getVertexCount(),
			instanceCount, 0, startInstanceLocation
		);
	}
}


void Mesh::drawIndexdInstanced(dx12lib::GraphicsContextProxy pGrahpicsCtx,
	size_t instanceCount /*= 1*/,
	size_t startInstanceLocation /*= 0 */) const
{
	if (!_subMeshs.empty()) {
		for (auto &submesh : *this)
			submesh.drawIndexdInstanced(pGrahpicsCtx, instanceCount, startInstanceLocation);
	} else {
		pGrahpicsCtx->drawIndexedInstanced(
			_pIndexBuffer->getIndexCount(),
			instanceCount, 0, 0, startInstanceLocation
		);
	}
}

SubMesh Mesh::getSubmesh(const std::string &name) const {
	SubMesh res;
	for (const auto &submesh : *this) {
		if (submesh._name == name) {
			res = submesh;
			break;
		}
	}
	return res;
}

const DX::BoundingBox& Mesh::getBounds() const {
	return _bounds;
}

}
