#include "MeshModel.h"
#include "MeshNode.h"

namespace d3d {

MeshModel::MeshModel(dx12lib::IDirectContext &directCtx, std::shared_ptr<ALTree> pALTree)
: _modelTransform(float4x4::identity())
, _pRootNode(std::make_unique<MeshNode>(directCtx, pALTree->getRootNode()))
{
	_pALTree = std::move(pALTree);
}

MeshModel::~MeshModel() = default;

void MeshModel::submit(const Frustum &frustum, const rg::TechniqueFlag &techniqueFlag) const {
	if (_modelTransformDirty) {
		_pRootNode->setParentTransform(Matrix4(_modelTransform));
		_modelTransformDirty = false;
	}
	_pRootNode->submit(frustum, techniqueFlag);
}

INode * MeshModel::getRootNode() const {
	return _pRootNode.get();
}

void MeshModel::setModelTransform(const float4x4 &matWorld) {
	_modelTransform = matWorld;
	_modelTransformDirty = true;
}

void MeshModel::createMaterial(const MaterialCreator &creator) {
	_pRootNode->createMaterial(creator);
}


}
