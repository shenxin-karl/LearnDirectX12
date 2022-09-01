#pragma once
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
	std::shared_ptr<dx12lib::IShaderResourceCube> pEnvMap;
private:
	std::shared_ptr<dx12lib::VertexBuffer> _pCubeVertexBuffer;
	std::shared_ptr<dx12lib::RootSignature> _pRootSignature;
	std::map<std::string, std::shared_ptr<dx12lib::GraphicsPSO>> _psoMap;
};

}
