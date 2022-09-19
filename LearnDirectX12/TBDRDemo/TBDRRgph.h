#pragma once
#include <RenderGraph/Pass/GraphicsPass.h>
#include <RenderGraph/Technique/TechniqueType.hpp>
#include <RenderGraph/Pass/ComputablePass.h>

#include "RenderGraph/Pass/FullScreenPass.h"
#include "RenderGraph/Pass/RenderQueuePass.h"

class TBDRApp;

namespace TBDRRgph {

constexpr rgph::TechniqueType kGBuffer{ 1 };
inline std::string ClearDsPass						= "ClearDsPass";
inline std::string ClearGBuffer						= "ClearGBuffer";
inline std::string GBuffer							= "GBuffer";
inline std::string Lighting							= "Lighting";
inline std::string LightingCopyToMainRt				= "LightingCopyToMainRt";
inline std::string SkyBox							= "SkyBox";
inline std::string ColorGradingAndGammaCorrection	= "ColorGradingAndGammaCorrection";
inline std::string MainRtCopyToBackRt				= "MainRtCopyToBackRt";
inline std::string Present							= "Present";


class ClearGBufferPass : public rgph::ExecutablePass {
public:
	ClearGBufferPass(const std::string &passName);
	void execute(dx12lib::DirectContextProxy pDirectCtx) override;
public:
	rgph::PassResourcePtr<dx12lib::IRenderTarget2D> pGBuffer0;
	rgph::PassResourcePtr<dx12lib::IRenderTarget2D> pGBuffer1;
	rgph::PassResourcePtr<dx12lib::IRenderTarget2D> pGBuffer2;
};

class GBufferPass : public rgph::RenderQueuePass {
	GBufferPass(const std::string &passName);
	void execute(dx12lib::DirectContextProxy pDirectCtx) override;
public:
	rgph::PassResourcePtr<dx12lib::IRenderTarget2D> pGBuffer0;
	rgph::PassResourcePtr<dx12lib::IRenderTarget2D> pGBuffer1;
	rgph::PassResourcePtr<dx12lib::IRenderTarget2D> pGBuffer2;
};

class LightingPass : public rgph::ComputablePass {
public:
	LightingPass(const std::string &passName);
public:
	rgph::PassResourcePtr<dx12lib::IRenderTarget2D> pGBuffer0;
	rgph::PassResourcePtr<dx12lib::IRenderTarget2D> pGBuffer1;
	rgph::PassResourcePtr<dx12lib::IRenderTarget2D> pGBuffer2;
	rgph::PassResourcePtr<dx12lib::IDepthStencil2D> pDepthBuffer;
	rgph::PassResourcePtr<dx12lib::IUnorderedAccess2D> pLightingBuffer;
};

class PostPass : public rgph::FullScreenPass {
public:
	using rgph::FullScreenPass::FullScreenPass;
	rgph::PassResourcePtr<dx12lib::IRenderTarget2D> pBackBuffer;
};

std::shared_ptr<rgph::RenderGraph> createTBDRRenderGraph(TBDRApp *pApp);

}
