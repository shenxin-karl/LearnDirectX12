#include "Mesh.h"
#include <dx12lib/Buffer/VertexBuffer.h>
#include <dx12lib/Buffer/IndexBuffer.h>

namespace d3d {

SubMesh::operator bool() const {
	return count != -1;
}

void SubMesh::drawInstanced(dx12lib::GraphicsContextProxy pGraphicsCtx,
	size_t instanceCount /*= 1*/,
	size_t startInstanceLocation /*= 0 */) const {
	assert(static_cast<bool>(*this) && "this submesh invalid");
	pGraphicsCtx->drawInstanced(
		count,
		instanceCount,
		baseVertexLocation,
		startInstanceLocation
	);
}

void SubMesh::drawIndexedInstanced(dx12lib::GraphicsContextProxy pGraphicsCtx,
	size_t instanceCount /*= 1*/,
	size_t startInstanceLocation /*= 0 */) const
{
	assert(bool(*this) && "this submesh invalid");
	pGraphicsCtx->drawIndexedInstanced(
		count,
		instanceCount,
		startIndexLocation,
		baseVertexLocation,
		startInstanceLocation
	);
}

Mesh::Mesh(std::shared_ptr<dx12lib::VertexBuffer> pVertexBuffer, 
	std::shared_ptr<dx12lib::IndexBuffer> pIndexBuffer,
	const DX::BoundingBox &bounds,
	const std::vector<SubMesh> &subMeshs) 
: _pVertexBuffer(pVertexBuffer), _pIndexBuffer(pIndexBuffer), _subMeshs(subMeshs), _bounds(bounds)
{
	if (subMeshs.empty())
		_subMeshs.push_back(getSubMesh());
}

void Mesh::appendSubMesh(const SubMesh &subMesh) {
	_subMeshs.emplace_back(subMesh);
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


void Mesh::drawInstanced(dx12lib::GraphicsContextProxy pGraphicsCtx,
	size_t instanceCount /*= 1*/,
	size_t startInstanceLocation /*= 0 */) const {
	for (auto &subMesh : *this)
		subMesh.drawInstanced(pGraphicsCtx, instanceCount, startInstanceLocation);
}


void Mesh::drawIndexedInstanced(dx12lib::GraphicsContextProxy pGraphicsCtx,
	size_t instanceCount /*= 1*/,
	size_t startInstanceLocation /*= 0 */) const
{
	for (auto &subMesh : *this)
		subMesh.drawIndexedInstanced(pGraphicsCtx, instanceCount, startInstanceLocation);
}

SubMesh Mesh::getSubMesh(const std::string &name) const {
	SubMesh res;
	for (const auto &submesh : *this) {
		if (submesh.name == name) {
			res = submesh;
			break;
		}
	}
	return res;
}

SubMesh Mesh::getSubMesh() const {
	assert(_pVertexBuffer != nullptr || _pIndexBuffer != nullptr);
	SubMesh subMesh;
	subMesh.name = "MeshAllSubMesh";
	subMesh.baseVertexLocation = 0;
	subMesh.startIndexLocation = 0;
	subMesh.count = (_pIndexBuffer != nullptr) ? _pIndexBuffer->getIndexCount() : _pVertexBuffer->getVertexCount();
	return subMesh;
}

const DX::BoundingBox& Mesh::getBounds() const {
	return _bounds;
}

}
