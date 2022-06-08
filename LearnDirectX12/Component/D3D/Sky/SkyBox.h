#pragma once
#include "D3D/d3dutil.h"
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Context/ContextProxy.hpp>

namespace d3d {

using namespace Math;

struct SkyBoxDesc {
	dx12lib::GraphicsContextProxy pGraphicsCtx;
	std::wstring filename;
	std::shared_ptr<dx12lib::IShaderResourceCube> pCubeMap;
	DXGI_FORMAT renderTargetFormat;
	DXGI_FORMAT depthStencilFormat;
	bool enableGammaCorrection = false;
	bool enableToneMapping = false;
};

class SkyBox {
public:
	SkyBox(const SkyBoxDesc &desc);
	void render(dx12lib::GraphicsContextProxy pGraphicsCtx, std::shared_ptr<CameraBase> pCamera) const;
	std::shared_ptr<dx12lib::IShaderResourceCube> getEnvironmentMap() const;
	void setEnvironmentMap(std::shared_ptr<dx12lib::IShaderResourceCube> pCubeMap);
private:
	void buildCubeVertexBuffer(dx12lib::GraphicsContextProxy pGraphicsCtx);
private:
	FRConstantBufferPtr<float4x4> _pViewProj;
	std::shared_ptr<dx12lib::GraphicsPSO> _pSkyBoxPSO;
	std::shared_ptr<dx12lib::VertexBuffer> _pCubeVertexBuffer;
	std::shared_ptr<dx12lib::IShaderResourceCube> _pCubeMap;
};

}
