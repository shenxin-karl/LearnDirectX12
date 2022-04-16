#pragma once
#include "D3D/d3dutil.h"
#include "dx12lib/ContextProxy.hpp"
#include "dx12lib/dx12libStd.h"
#include "Math/MatrixHelper.h"

namespace d3d {

using namespace Math;

struct SkyBoxDesc {
	dx12lib::GraphicsContextProxy pGraphicsCtx;
	const std::wstring &filename;
	std::shared_ptr<dx12lib::ShaderResourceBuffer> pCubeMap;
	DXGI_FORMAT renderTargetFormat;
	DXGI_FORMAT depthStencilFormat;
};

class SkyBox {
public:
	SkyBox(const SkyBoxDesc &desc);
	void render(dx12lib::GraphicsContextProxy pGraphicsCtx, std::shared_ptr<CameraBase> pCamera);
private:
	enum RootParam : size_t {
		CB_Setting = 0,
		SR_CubeMap = 1,
	};
	void buildCubeVertexBuffer(dx12lib::GraphicsContextProxy pGraphicsCtx);
private:
	FRConstantBufferPtr<float4x4> _pViewProj;
	std::shared_ptr<dx12lib::GraphicsPSO> _pSkyBoxPSO;
	std::shared_ptr<dx12lib::VertexBuffer> _pCubeVertexBuffer;
	std::shared_ptr<dx12lib::ShaderResourceBuffer> _pCubeMap;
};

}
