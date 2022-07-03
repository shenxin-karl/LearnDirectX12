#include "StaticGeometryInput.h"

namespace d3d {
StaticGeometryInput::StaticGeometryInput(const AssimpLoader::ALMesh &mesh) {

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