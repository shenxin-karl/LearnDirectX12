#pragma once
#include "D3D/Model/MeshModel/MeshModel.h"

namespace d3d {

class ALNode;
class MeshNode : public INode {
public:
	MeshNode(dx12lib::IDirectContext &directCtx, const ALNode *pALNode);
	void submit(const rg::TechniqueFlag &techniqueFlag) const override;
	size_t getNumRenderItem() const override;
	RenderItem *getRenderItem(size_t idx) const override;
	void setParentTransform(const Matrix4 &matWorld) override;
	FRConstantBufferPtr<NodeTransform> getNodeTransform() const override;
private:
	mutable bool _transformDirty = true;
	float4x4 _applyTransform;
	float4x4 _nodeLocalTransform;
	std::vector<std::unique_ptr<RenderItem>> _renderItems;
	std::vector<std::unique_ptr<MeshNode>> _children;
	FRConstantBufferPtr<NodeTransform> _pNodeTransform;
};

}
