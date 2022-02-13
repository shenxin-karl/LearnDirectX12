#pragma once
#include "BaseApp/BaseApp.h"
#include "GameTimer/GameTimer.h"
#include "Math/VectorHelper.h"
#include "Math/MatrixHelper.h"
#include "D3D/FrameResource.h"
#include "dx12lib/StructConstantBuffer.hpp"

using namespace vec;
using namespace mat;

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

class BoxApp : public com::BaseApp {
public:
	BoxApp();
protected:
	virtual void onInitialize(dx12lib::CommandListProxy pCmdList) override;
	virtual void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
private:
	void buildBoxGeometry(dx12lib::CommandListProxy pCmdList);
	void renderBoxPass(dx12lib::CommandListProxy pCmdList);
	using GPUPassConstantBufferPtr = std::shared_ptr<dx12lib::StructConstantBuffer<d3dUtil::PassConstants>>;
private:
	std::shared_ptr<dx12lib::GraphicsPSO>  _pGraphicsPSO;
	GPUPassConstantBufferPtr               _pPassConstantBuffer;
	std::unique_ptr<BoxMesh>               _pBoxMesh;
	float    _theta = 0.f;
	float    _phi = 0.f;
	float4x4 _world;
	float4x4 _projection;
	float4x4 _view;
};