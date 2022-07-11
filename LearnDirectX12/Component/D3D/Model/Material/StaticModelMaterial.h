#pragma once
#include <D3D/Model/ModelInterface.hpp>
#include <dx12lib/Texture/TextureStd.h>
#include <D3D/Shader/ShaderCommon.h>

namespace d3d {
using namespace Math;

struct StaticModelCBType {
	float4x4	 matWorld = float4x4::identity();
	float4x4	 matNormal = float4x4::identity();
	float4x4	 matTexCoord = float4x4::identity();
	MaterialData materialData = MaterialData::defaultMaterialData;
};

class StaticModelMaterial : public IMaterial {
public:
	StaticModelMaterial() = default;
	const std::string &getPSOName() const override;
	void active(dx12lib::GraphicsContextProxy pGraphicsCtx) override;
	static void initPSOCreator();
private:
	static inline std::string psoName = "StaticModel";
	std::shared_ptr<dx12lib::IShaderResource> _pAoMap;
	std::shared_ptr<dx12lib::IShaderResource> _pAlbedoMap;
	std::shared_ptr<dx12lib::IShaderResource> _pNormalMap;
	std::shared_ptr<dx12lib::IShaderResource> _pMetallicMap;
	std::shared_ptr<dx12lib::IShaderResource> _pRoughnessMap;
	dx12lib::FRConstantBufferPtr<StaticModelCBType> _pObjectCb;
};

class StaticModelShadowMaterial : public IMaterial {
public:
	const std::string &getPSOName() const override;
	void active(dx12lib::GraphicsContextProxy pGraphicsCtx) override;
private:
	dx12lib::FRConstantBufferPtr<StaticModelCBType> _pObjectCb;
};

}
