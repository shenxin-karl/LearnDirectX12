#include "StaticGeometryInput.h"

namespace d3d {
StaticGeometryInput::StaticGeometryInput(dx12lib::GraphicsContextProxy pGraphicsCtx, const AssimpLoader::ALMesh &mesh) {
	_pVertexBuffer = pGraphicsCtx->createVertexBuffer(mesh.vertices.data(), 
		mesh.vertices.size(), 
		sizeof(com::Vertex)
	);
	_pIndexBuffer = pGraphicsCtx->createIndexBuffer(mesh.indices.data(), 
		mesh.indices.size(), 
		DXGI_FORMAT_R16_UINT
	);
}

void StaticGeometryInput::bind(dx12lib::GraphicsContextProxy pGraphicsCtx) {
	pGraphicsCtx->setVertexBuffer(_pVertexBuffer);
	pGraphicsCtx->setIndexBuffer(_pIndexBuffer);
}

std::shared_ptr<dx12lib::IndexBuffer> StaticGeometryInput::getIndexBuffer() const {
	return _pIndexBuffer;
}

std::shared_ptr<dx12lib::VertexBuffer> StaticGeometryInput::getVertexBuffer() const {
	return _pVertexBuffer;
}

}