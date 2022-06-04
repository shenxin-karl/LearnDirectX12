#pragma once
#include "dx12lib/dx12libStd.h"
#include "dx12lib/Context/ContextProxy.hpp"
#include "BaseApp/BaseApp.h"
#include "GameTimer/GameTimer.h"

#include "D3D/d3dutil.h"
#include "D3D/Tool/Mesh.h"
#include "D3D/Tool/Camera.h"
#include "D3D/Shader/ShaderCommon.h"

using namespace Math;

enum RenderLayer : std::size_t {
	Opaque = 0,
	Mirrors,
	Reflected,
	Transparent,
	Shadow,
	Count,
};

struct ObjectCBType {
	float4x4      matWorld;
	float4x4      matNormal;
	d3d::Material material;
};

struct RenderItem {
	std::shared_ptr<d3d::Mesh> _pMesh;
	dx12lib::FRConstantBufferPtr<ObjectCBType> _pObjectCB;
	std::shared_ptr<dx12lib::IShaderResource> _pAlbedoMap;
	d3d::SubMesh _submesh;
};

struct Vertex {
	float3 position;
	float3 normal;
	float2 texcoord;
public:
	Vertex(const com::Vertex &vertex);
	Vertex(const float3 &position, const float3 &normal, const float2 &texcoord);
};

class MirrorApp : public com::BaseApp {
public:
	MirrorApp();
	~MirrorApp();
public:
	void onInitialize(dx12lib::DirectContextProxy pDirectCtx) override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) override;
private:
	void drawRenderItems(dx12lib::DirectContextProxy pDirectCtx, RenderLayer layer);
	void buildCamera();
	void buildConstantBuffers(dx12lib::DirectContextProxy pDirectCtx);
	void loadTextures(dx12lib::DirectContextProxy pDirectCtx);
	void buildMaterials();
	void buildMeshs(dx12lib::DirectContextProxy pDirectCtx);
	void buildPSOs(dx12lib::DirectContextProxy pDirectCtx);
	void buildRenderItems(dx12lib::DirectContextProxy pDirectCtx);
private:
	std::unique_ptr<d3d::FXAA>		   _pFXAAFilter;
	std::unique_ptr<d3d::CoronaCamera> _pCamera;
	FRConstantBufferPtr<d3d::CBPassType>  _pPassCB;
	FRConstantBufferPtr<d3d::CBLightType> _pLightCB;
	FRConstantBufferPtr<d3d::CBLightType> _pReflectedLightCB;
	std::map<std::string, d3d::Material> _materialMap;
	std::map<std::string, std::shared_ptr<d3d::Mesh>> _meshMap;
	std::map<RenderLayer, std::shared_ptr<dx12lib::GraphicsPSO>> _psoMap;
	std::map<std::string, std::shared_ptr<dx12lib::IShaderResource>> _textureMap;
	std::vector<RenderItem> _renderItems[RenderLayer::Count];

	float3 _skullTranslation = { 0.0f, 1.0f, -5.0f };
	dx12lib::FRConstantBufferPtr<ObjectCBType> _pSkullObjectCB;
};