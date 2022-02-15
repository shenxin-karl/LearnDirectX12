#pragma once
#include "BaseApp/BaseApp.h"
#include "GameTimer/GameTimer.h"
#include "Math/MathHelper.h"
#include "dx12lib/StructConstantBuffer.hpp"

using namespace Math;

struct Vertex {
	float3 position;
	float4 color;
};

struct BoxMesh {
	std::shared_ptr<dx12lib::VertexBuffer> _pVertexBuffer;
	std::shared_ptr<dx12lib::IndexBuffer>  _pIndexBuffer;
	UINT _baseVertexLocation = 0;
	UINT _startIndexLocation = 0;
};

enum BoxRootParame : UINT {
	WorldViewProjCBuffer = 0,
};

struct WVMConstantBuffer {
	float4x4 gWorldViewProj;
};

class BoxApp : public com::BaseApp {
public:
	BoxApp();
protected:
	virtual void onInitialize(dx12lib::CommandListProxy pCmdList) override;
	virtual void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
private:
	void pollEvent();
	void updatePhiAndTheta(int x, int y);
	void updateRadius(float offset);
	void buildBoxGeometry(dx12lib::CommandListProxy pCmdList);
	void renderBoxPass(dx12lib::CommandListProxy pCmdList);
	using GPUPassConstantBufferPtr = std::shared_ptr<dx12lib::StructConstantBuffer<WVMConstantBuffer>>;
private:
	std::shared_ptr<dx12lib::GraphicsPSO>  _pGraphicsPSO;
	GPUPassConstantBufferPtr               _pMVPConstantBuffer;
	std::unique_ptr<BoxMesh>               _pBoxMesh;
	float    _theta = 0.f;
	float    _phi = 0.f;
	float    _radius = 5.f;
	bool     _isMouseLeftPress = false;
	POINT    _lastMousePosition;
};