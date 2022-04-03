#pragma once
#include "BaseApp/BaseApp.h"
#include "D3D/d3dutil.h"
#include "dx12lib/dx12libStd.h"
#include "dx12lib/StructuredConstantBuffer.hpp"
#include "Math/MathHelper.h"

using namespace Math;

struct PatchVertex {
	float3 position;
};

enum RootParame {
	CB_Object,
	CB_Pass,
};

class BasicTessellationApp : public com::BaseApp {
public:
	BasicTessellationApp();
	~BasicTessellationApp() override;
protected:
	void onInitialize(dx12lib::DirectContextProxy pDirectCtx) override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onResize(dx12lib::DirectContextProxy pCmdList, int width, int height) override;
private:
	std::unique_ptr<d3d::CoronaCamera>    _pCamera;
	GPUStructuredCBPtr<d3d::PassCBType>		  _pPassCB;
	GPUStructuredCBPtr<float4x4>			  _pObjectCB;
	std::shared_ptr<dx12lib::GraphicsPSO> _pTessellationPSO;
	std::shared_ptr<d3d::Mesh>			  _pQuadMesh;
};