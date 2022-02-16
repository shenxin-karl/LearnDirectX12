#pragma once
#include "dx12libStd.h"
#include "GameTimer/GameTimer.h"
#include "BaseApp/BaseApp.h"
#include "D3D/d3dutil.h"
#include "Math/MathHelper.h"
#include "dx12lib/StructConstantBuffer.hpp"

using namespace Math;

struct Vertex {
	float3 position;
	float3 normal;
};

struct Mesh {
	std::shared_ptr<dx12lib::VertexBuffer> _pVertexBuffer;
	std::shared_ptr<dx12lib::IndexBuffer>  _pIndexBuffer;
};

struct ObjectCB {
	float4x4 gWorld;
	float3   albedo;
	float    roughness;
	float3   fresnel;
	float    metallic;
};

struct RenderItem {
	UINT _baseVertexLocation;
	UINT _startIndexLocation;
	UINT _indexCount;
	std::shared_ptr<Mesh>     _pMesh;
	std::shared_ptr<ObjectCB> _pObjectCB;
};

enum ShapeShaderCBType : UINT {
	Pass   = 0,
	Light  = 1,
	Object = 2,
};

class Shape : public com::BaseApp {
public:
	Shape();
	virtual ~Shape() override;
	virtual void onInitialize(dx12lib::CommandListProxy pCmdList) override;
	virtual void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
private:
	using GPUGameLightCBPtr = std::shared_ptr<dx12lib::StructConstantBuffer<d3dutil::LightCBType>>;
	using GPUPassCBPtr = std::shared_ptr<dx12lib::StructConstantBuffer<d3dutil::PassCBType>>;
	void buildPSO(dx12lib::CommandListProxy pCmdList);
	void buildRenderItem(dx12lib::CommandListProxy pCmdList);
	void renderShapesPass(dx12lib::CommandListProxy pCmdList);
	void pollEvent();
private:
	POINT _lastMousePoint;
	bool  _isLeftPress = false;
	std::unique_ptr<d3dutil::CoronaCamera>  _pCamera;
	std::shared_ptr<dx12lib::GraphicsPSO>   _pGraphicsPSO;
	GPUGameLightCBPtr                       _pGameLightsCB;
	GPUPassCBPtr                            _pPassCB;
	std::vector<RenderItem>                 _renderItems;
};
