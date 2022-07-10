#include "Drawable.h"
#include <RenderGraph/Technique/Technique.h>

namespace rg {

void Drawable::bind(dx12lib::GraphicsContextProxy &pGraphicsCtx) {
	assert(_topology != D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);
	pGraphicsCtx->setPrimitiveTopology(_topology);
	if (_pVertexBuffer != nullptr)
		pGraphicsCtx->setVertexBuffer(_pVertexBuffer);
	if (_pIndexBuffer != nullptr)
		pGraphicsCtx->setIndexBuffer(_pIndexBuffer);
}

void Drawable::submit(TechniqueType filter) {
	for (auto &pTechnique : _techniques)
		pTechnique->submit(*this, filter);
}

void Drawable::addTechnique(std::unique_ptr<Technique> pTechnique) {
	
}

void Drawable::remoteTechnique(const std::string &techniqueName) {
}

TechniqueType * Drawable::getTechnique(const std::string &techniqueName) const {
}

void Drawable::setDrawArgs(const DrawArgs &drawArgs) {
	_drawArgs = drawArgs;
}

void Drawable::setTopology(D3D_PRIMITIVE_TOPOLOGY topology) {
	_topology = topology;
}

void Drawable::setIndexBuffer(std::shared_ptr<dx12lib::IndexBuffer> pIndexBuffer) {
	_pIndexBuffer = std::move(pIndexBuffer);
}

void Drawable::setVertexBuffer(std::shared_ptr<dx12lib::VertexBuffer> pVertexBuffer) {
	_pVertexBuffer = std::move(pVertexBuffer);
}

const DrawArgs & Drawable::getDrawArgs() const {
	return _drawArgs;
}

D3D_PRIMITIVE_TOPOLOGY Drawable::getTopology() const {
	return _topology;
}

std::shared_ptr<dx12lib::IndexBuffer> Drawable::getIndexBuffer() const {
	return _pIndexBuffer;
}

std::shared_ptr<dx12lib::VertexBuffer> Drawable::getVertexBuffer() const {
	return _pVertexBuffer;
}

}
