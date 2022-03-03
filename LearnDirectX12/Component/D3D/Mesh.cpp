#include "Mesh.h"


namespace d3d {

SubMesh::operator bool() const {
	return _count != -1;
}

void SubMesh::drawInstanced(dx12lib::CommandListProxy pCmdList, 
	std::uint32_t instanceCount /*= 1*/, 
	std::uint32_t startInstanceLocation /*= 0 */) const 
{
	assert(bool(*this) && "this submesh invalid");
	pCmdList->drawInstanced(
		_count,
		instanceCount,
		_baseVertexLocation,
		startInstanceLocation
	);
}

void SubMesh::drawIndexdInstanced(dx12lib::CommandListProxy pCmdList, 
	std::uint32_t instanceCount /*= 1*/, 
	std::uint32_t startInstanceLocation /*= 0 */) const 
{
	assert(bool(*this) && "this submesh invalid");
	pCmdList->drawIndexdInstanced(
		_count,
		instanceCount,
		_startIndexLocation,
		_baseVertexLocation,
		startInstanceLocation
	);
}

Mesh::Mesh(std::shared_ptr<dx12lib::VertexBuffer> pVertexBuffer, 
	std::shared_ptr<dx12lib::IndexBuffer> pIndexBuffer, 
	const std::vector<SubMesh> &subMeshs) 
: _pVertexBuffer(pVertexBuffer), _pIndexBuffer(pIndexBuffer), _subMeshs(subMeshs) {
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


void Mesh::drawInstanced(dx12lib::CommandListProxy pCmdList, 
	std::uint32_t instanceCount /*= 1*/, 
	std::uint32_t startInstanceLocation /*= 0 */) const 
{
	for (auto &submesh : *this)
		submesh.drawInstanced(pCmdList, instanceCount, startInstanceLocation);
}


void Mesh::drawIndexdInstanced(dx12lib::CommandListProxy pCmdList, 
	std::uint32_t instanceCount /*= 1*/, 
	std::uint32_t startInstanceLocation /*= 0 */) const 
{
	for (auto &submesh : *this)
		submesh.drawIndexdInstanced(pCmdList, instanceCount, startInstanceLocation);
}

}
