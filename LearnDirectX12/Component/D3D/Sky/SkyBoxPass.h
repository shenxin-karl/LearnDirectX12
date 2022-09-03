#pragma once
#include <D3D/d3dutil.h>
#include <RenderGraph/Pass/GraphicsPass.h>

namespace d3d {

class SkyBoxPass : public rgph::GraphicsPass {
public:
	SkyBoxPass(const std::string &passName);
	void execute(dx12lib::DirectContextProxy pDirectCtx) override;
public:
	bool enableToneMapping = false;
	bool enableGammaCorrection = false;
	DXGI_FORMAT renderTargetFormat = DXGI_FORMAT_UNKNOWN;
	DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_UNKNOWN;
	std::shared_ptr<dx12lib::ITextureResourceCube> pEnvMap;
	CameraBase *pCamera = nullptr;
private:
	void buildGraphicsPso(dx12lib::DirectContextProxy pDirectCtx);
private:
	std::shared_ptr<dx12lib::GraphicsPSO> _pGraphicsPso;
	std::shared_ptr<dx12lib::VertexBuffer> _pCubeVertexBuffer;
	std::shared_ptr<dx12lib::RootSignature> _pRootSignature;
};

}
