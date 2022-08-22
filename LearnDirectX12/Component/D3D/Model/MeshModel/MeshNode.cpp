#include "MeshNode.h"
#include "D3D/AssimpLoader/ALNode.h"
#include "D3D/Model/RenderItem/RenderItem.h"

namespace d3d {

MeshNode::MeshNode(dx12lib::IDirectContext &directCtx, const ALNode *pALNode) {
	_applyTransform = pALNode->getNodeTransform();
	_nodeLocalTransform = pALNode->getNodeTransform();

	for (size_t i = 0; i < pALNode->getNumMesh(); ++i) 
		_renderItems.emplace_back(std::make_unique<RenderItem>(directCtx, pALNode->getMesh(i)));

	for (size_t i = 0; i < pALNode->getNumChildren(); ++i)
		_children.push_back(std::make_unique<MeshNode>(directCtx, pALNode->getChildren(i)));

	_pNodeTransform = directCtx.createFRConstantBuffer<NodeTransform>();
}

void MeshNode::submit(const rg::TechniqueFlag &techniqueFlag) const {
	if (_renderItems.empty())
		return;

	if (_transformDirty) {
		auto visitor = _pNodeTransform->visit();
		visitor->matWorld = _applyTransform;
		visitor->matNormal = float4x4(transpose(inverse(Matrix4(_applyTransform))));
		_transformDirty = false;
	}

	for (auto &pRenderItem : _renderItems)
		pRenderItem->submit(techniqueFlag);

	for (auto &pChild : _children)
		pChild->submit(techniqueFlag);
}

size_t MeshNode::getNumRenderItem() const {
	return _renderItems.size();
}

RenderItem * MeshNode::getRenderItem(size_t idx) const {
	if (idx >= getNumRenderItem()) {
		assert(false);
		return nullptr;
	}
	return _renderItems[idx].get();
}

void MeshNode::setParentTransform(const Matrix4 &matWorld) {
	Matrix4 localTransform(_nodeLocalTransform);
	Matrix4 applyTransform = localTransform * matWorld;
	_applyTransform = float4x4(applyTransform);
	_transformDirty = true;
	for (auto &pChildren : _children)
		pChildren->setParentTransform(applyTransform);
}

FRConstantBufferPtr<NodeTransform> MeshNode::getNodeTransform() const {
	return _pNodeTransform;
}

}
