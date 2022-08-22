#pragma once
#include "D3D/AssimpLoader/ALTree.h"
#include "D3D/Model/IModel.hpp"
#include "D3D/Model/RenderItem/RenderItem.h"

namespace d3d {

class MeshNode;
class Material;
class MeshModel : public IModel {
public:
	MeshModel(dx12lib::IDirectContext &directCtx, std::shared_ptr<ALTree> pALTree);
	void submit(const rg::TechniqueFlag &techniqueFlag) const override ;
	INode *getRootNode() const override;
	void setModelTransform(const float4x4 &matWorld) override;

	using MaterialCreator = std::function<std::shared_ptr<Material>(MeshNode *, RenderItem *)>;
	void createMaterial(const MaterialCreator &creator);
private:
	mutable bool _modelTransformDirty = true;
	float4x4 _modelTransform;
	std::unique_ptr<MeshNode> _pRootNode;
};

}
