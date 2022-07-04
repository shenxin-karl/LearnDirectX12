#include "RenderItem.h"
#include "D3D/Model/ModelInterface.hpp"

namespace d3d {

RenderItem::RenderItem(const std::source_location &sr) : _sourceLocation(sr) {
}

void RenderItem::setPassName(const std::string &passName) {
	_passName = passName;
}

void RenderItem::setGeometryInput(std::shared_ptr<IGeometryInput> pRenderInput) {
	_pGeometryInput = pRenderInput;
}

void RenderItem::setInstanceInput(std::shared_ptr<IInstanceInput> pInstanceInput) {
	_pInstanceInput = pInstanceInput;
}

void RenderItem::setMaterial(std::shared_ptr<IMaterial> pMaterial) {
	_pMaterial = pMaterial;
}

void RenderItem::setDrawArgs(const DrawArgs &drawArgs) {
	_drawArgs = drawArgs;
}

const std::string &RenderItem::getPSOName() const {
	return _pMaterial->getPSOName();
}

const std::string &RenderItem::getPassName() const {
	return _passName;
}

std::shared_ptr<IGeometryInput> RenderItem::getGeometryInput() const {
	return _pGeometryInput;
}

std::shared_ptr<IInstanceInput> RenderItem::getInstanceInput() const {
	return _pInstanceInput;
}

std::shared_ptr<IMaterial> RenderItem::getMaterial() const {
	return _pMaterial;
}

const DrawArgs &RenderItem::getDrawArgs() const {
	return _drawArgs;
}

void RenderItem::draw(dx12lib::GraphicsContextProxy pGraphicsCtx) const {
	_pGeometryInput->bind(pGraphicsCtx);
	_pMaterial->active(pGraphicsCtx);
	if (_drawArgs.instanceCount > 1)
		_pInstanceInput->bind(pGraphicsCtx);

	if (_drawArgs.indexCount > 0) {
		pGraphicsCtx->drawIndexedInstanced(
			_drawArgs.indexCount,
			_drawArgs.instanceCount,
			_drawArgs.startIndexLocation,
			_drawArgs.baseVertexLocation,
			_drawArgs.startInstanceLocation
		);
	} else {
		pGraphicsCtx->drawInstanced(
			_drawArgs.vertexCount,
			_drawArgs.instanceCount,
			_drawArgs.baseVertexLocation,
			_drawArgs.startInstanceLocation
		);
	}
}

}