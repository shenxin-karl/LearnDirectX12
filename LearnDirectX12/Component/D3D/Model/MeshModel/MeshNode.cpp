#include "MeshNode.h"
#include "D3D/AssimpLoader/ALNode.h"
#include "D3D/Model/RenderItem/RenderItem.h"

namespace d3d {

MeshNode::MeshNode(dx12lib::IDirectContext &directCtx, const ALNode *pALNode) {
	_applyTransform = pALNode->getNodeTransform();
	_nodeLocalTransform = pALNode->getNodeTransform();
	_nodeTransformCBuffer.setTransformCBuffer(directCtx.createFRConstantBuffer<rgph::TransformStore>());

	for (size_t i = 0; i < pALNode->getNumMesh(); ++i) {
		_alMeshes.push_back(pALNode->getMesh(i));
		_renderItems.emplace_back(std::make_unique<RenderItem>(directCtx, this, i));
	}

	for (size_t i = 0; i < pALNode->getNumChildren(); ++i)
		_children.push_back(std::make_unique<MeshNode>(directCtx, pALNode->getChildren(i)));
}

void MeshNode::submit(const Frustum &frustum, const rgph::TechniqueFlag &techniqueFlag) const {
	if (_renderItems.empty())
		return;

	if (_transformDirty) {
		rgph::TransformStore store {
			.matWorld = _applyTransform,
			.matNormal = float4x4(transpose(inverse(Matrix4(_applyTransform))))
		};
		_nodeTransformCBuffer.setTransformStore(store);
		_transformDirty = false;
	}

	for (auto &pRenderItem : _renderItems) {
		const auto &worldAABB = pRenderItem->getWorldAABB();
		if (frustum.contains(worldAABB) == DX::ContainmentType::DISJOINT)
			continue;
		pRenderItem->submit(techniqueFlag);
	}

	for (auto &pChild : _children)
		pChild->submit(frustum, techniqueFlag);
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

	for (auto &pRenderItem : _renderItems)
		pRenderItem->applyTransform(applyTransform);
}

const rgph::TransformCBufferPtr &MeshNode::getNodeTransformCBuffer() const {
	return _nodeTransformCBuffer;
}

std::shared_ptr<rgph::IMesh> MeshNode::getMesh(size_t idx) const {
	assert(idx < _alMeshes.size());
	return _alMeshes[idx];
}

void MeshNode::createMaterial(rgph::RenderGraph &graph, const MeshModel::MaterialCreator &creator) {
	for (auto &pRenderItem : _renderItems) {
		pRenderItem->setMaterial(creator(this, pRenderItem.get()));
		pRenderItem->rebuildTechniqueFromMaterial();
	}
	for (auto &pChild : _children)
		pChild->createMaterial(graph, creator);
}

}
