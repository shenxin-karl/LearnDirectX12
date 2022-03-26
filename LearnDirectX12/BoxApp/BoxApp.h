#pragma once
#include "BaseApp/BaseApp.h"
#include "GameTimer/GameTimer.h"
#include "D3D/Camera.h"
#include "Math/MathHelper.h"
#include "dx12lib/StructuredConstantBuffer.hpp"

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
	virtual void onInitialize(dx12lib::DirectContextProxy pDirectContext) override;
	virtual void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
private:
	void pollEvent();
	void buildBoxGeometry(dx12lib::DirectContextProxy pDirectContext);
	void renderBoxPass(dx12lib::DirectContextProxy pDirectContext);
private:
	std::shared_ptr<dx12lib::GraphicsPSO>  _pGraphicsPSO;
	std::unique_ptr<d3d::CoronaCamera>     _pCamera;
	GPUStructCBPtr<WVMConstantBuffer>      _pMVPConstantBuffer;
	std::unique_ptr<BoxMesh>               _pBoxMesh;
	float    _theta = 0.f;
	float    _phi = 0.f;
	float    _radius = 5.f;
	bool     _isMouseLeftPress = false;
	POINT    _lastMousePosition;
};