#pragma once
#include <Math/MathStd.hpp>
#include <D3D/Shader/ShaderCommon.h>
#include <D3D/d3dutil.h>
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Context/ContextStd.h>
#include <dx12lib/Buffer/BufferStd.h>
#include "IRenderItem.h"

using namespace Math;

class Model {
public:
	struct CBObjectType;
	struct ModelRenderItem;
	struct ShadowRenderItem;
public:
	Model(dx12lib::DirectContextProxy pDirectCtx, const d3d::AssimpLoader &loader);
	std::shared_ptr<ModelRenderItem> getModelRenderItem() const;
	std::shared_ptr<ShadowRenderItem> getShadowRenderItem() const;
	CBObjectType &getCbObject();
	const CBObjectType &getCbObject() const;
private:
	std::shared_ptr<d3d::Mesh> _pMesh;
	dx12lib::FRConstantBufferPtr<CBObjectType> _pObjectCb;
	std::shared_ptr<ModelRenderItem> _pModelRenderItem;
	std::shared_ptr<ShadowRenderItem> _pShadowRenderItem;
	std::shared_ptr<dx12lib::SamplerTexture2D> _pDiffuseAlbedo;
};

struct Model::CBObjectType {
	float4x4 matWorld;
	float4x4 matNormal;
	float4x4 matTexCoord;
	d3d::Material gMaterial;
};

struct Model::ModelRenderItem final : public IRenderItem {
	void draw(dx12lib::DirectContextProxy pDirectCtx) override;
public:
	std::shared_ptr<d3d::Mesh> pMesh;
	dx12lib::FRConstantBufferPtr<CBObjectType> pObjectCb;
	std::shared_ptr<dx12lib::SamplerTexture2D> pDiffuseAlbedo;
};

struct Model::ShadowRenderItem final : public IRenderItem {
	void draw(dx12lib::DirectContextProxy pDirectCtx) override;
public:
	std::shared_ptr<d3d::Mesh> pMesh;
	dx12lib::FRConstantBufferPtr<CBObjectType> pObjectCb;
};