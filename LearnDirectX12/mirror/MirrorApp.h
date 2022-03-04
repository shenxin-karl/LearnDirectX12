#pragma once
#include "dx12lib/dx12libStd.h"
#include "dx12lib/StructConstantBuffer.hpp"
#include "dx12lib/CommandListProxy.h"
#include "BaseApp/BaseApp.h"
#include "GameTimer/GameTimer.h"
#include "D3D/d3dutil.h"
#include "D3D/Mesh.h"
#include "D3D/Camera.h"
#include "D3D/ShaderCommon.h"
#include "Math/MathHelper.h"

using namespace Math;

enum RenderLayer : std::size_t {
	Opaque = 0,
	Mirrors,
	Reflected,
	Transparent,
	Shadow,
	Count,
};

enum RootParame : std::uint32_t {
	CBObject = 0,
	SRAlbedo = 1,
	CBPass   = 2,
	CBLight  = 3,
};

struct ObjectCBType {
	float4x4      matWorld;
	float4x4      matNormal;
	d3d::Material material;
};

struct RenderItem {
	std::shared_ptr<d3d::Mesh> pMesh;
	GPUStructCBPtr<ObjectCBType> _pObjectCB;
	std::shared_ptr<dx12lib::Texture> _pAlbedoMap;
};

class MirrorApp : public com::BaseApp {
public:
	MirrorApp();
	~MirrorApp();
public:
	virtual void onInitialize(dx12lib::CommandListProxy pCmdList) override;
	virtual void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void onResize(dx12lib::CommandListProxy pCmdList, int width, int height) override;
private:
	void drawRenderItems(dx12lib::CommandListProxy pCmdList, RenderLayer layer);
	void buildCamera();
	void buildConstantBuffers(dx12lib::CommandListProxy pCmdList);
	void loadTextures(dx12lib::CommandListProxy pCmdList);
	void buildMaterials();
	void buildMeshs(dx12lib::CommandListProxy pCmdList);
	void buildPSOs();
	void buildRenderItems();
private:
	std::unique_ptr<d3d::CoronaCamera> _pCamera;
	GPUStructCBPtr<d3d::PassCBType>    _pPassCB;
	GPUStructCBPtr<d3d::LightCBType>   _pLightCB;
	std::map<std::string, d3d::Material> _materialMap;
	std::map<std::string, std::shared_ptr<d3d::Mesh>> _meshMap;
	std::map<std::string, std::shared_ptr<dx12lib::Texture>> _textureMap;
	std::map<RenderLayer, std::shared_ptr<dx12lib::GraphicsPSO>> _psoMap;
	std::vector<RenderItem> _renderItems[RenderLayer::Count];
};