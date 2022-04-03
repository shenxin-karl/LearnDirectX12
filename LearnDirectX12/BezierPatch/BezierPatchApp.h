#pragma once
#include "BaseApp/BaseApp.h"
#include "D3D/d3dutil.h"
#include "D3D/ShaderCommon.h"
#include "dx12lib/dx12libStd.h"
#include "dx12lib/StructuredConstantBuffer.hpp"
#include "Math/MathHelper.h"

using namespace Math;

struct PatchVertex {
	float3 position;
};

struct CBObject {
	float4x4      gWorld;
	d3d::Material gMaterial;
};

enum RootParame {
	CB_Object = 0,
	CB_Pass   = 1,
	CB_Light  = 2,
};

class BezierPatchApp : public com::BaseApp {
public:
	BezierPatchApp();
	~BezierPatchApp() override;
protected:
	void onInitialize(dx12lib::DirectContextProxy pDirectCtx) override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) override;
protected:
	void buildCamera();
	void buildConstantBuffer(dx12lib::CommandContextProxy pCmdCtx);
	void buildGeometry(dx12lib::GraphicsContextProxy pGraphicsCtx);
	void buildBezierPatchPSO();
private:
	std::unique_ptr<d3d::CoronaCamera>    _pCamara;
	GPUStructuredCBPtr<d3d::PassCBType>   _pPassCB;
	GPUStructuredCBPtr<d3d::LightCBType>  _pLightCB;
	GPUStructuredCBPtr<CBObject>		  _pObjectCB;
	std::unique_ptr<d3d::Mesh>			  _pQuadMesh;
	std::shared_ptr<dx12lib::GraphicsPSO> _pBezierPatchPSO;
};