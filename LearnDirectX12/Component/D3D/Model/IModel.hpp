#pragma once
#include <D3D/d3dutil.h>
#include <RenderGraph/Drawable/Drawable.h>
#include <RenderGraph/Bindable/ConstantBufferBindable.h>

namespace rgph {

struct IMesh;
class TransformCBufferPtr;

}

namespace d3d {

using namespace Math;
class RenderItem;

interface INode : protected NonCopyable {
	virtual void submit(const Frustum &frustum, const rgph::TechniqueFlag &techniqueFlag) const = 0;
	virtual size_t getNumRenderItem() const = 0;
	virtual RenderItem *getRenderItem(size_t idx) const = 0;
	virtual void setParentTransform(const Matrix4 &matWorld) = 0;
	virtual const rgph::TransformCBufferPtr &getNodeTransformCBuffer() const = 0;
	virtual std::shared_ptr<rgph::IMesh> getMesh(size_t idx) const = 0;
};

interface IModel : protected NonCopyable {
	virtual void submit(const Frustum &frustum, const rgph::TechniqueFlag &techniqueFlag) const = 0;
	virtual INode *getRootNode() const = 0;
	virtual void setModelTransform(const float4x4 &matWorld) = 0;
};

}
