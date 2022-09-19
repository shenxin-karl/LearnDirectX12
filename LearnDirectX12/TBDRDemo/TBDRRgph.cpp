#include "TBDRRgph.h"

#include "TBDRApp.h"
#include "D3D/Sky/SkyBoxPass.h"
#include "Dx12lib/Device/SwapChain.h"
#include "RenderGraph/Pass/PresentPass.hpp"
#include "Dx12lib/Texture/TextureStd.h"
#include "RenderGraph/Pass/ClearPass.hpp"
#include "RenderGraph/Pass/CopyPass.hpp"
#include "RenderGraph/RenderGraph/RenderGraph.h"

namespace TBDRRgph {

ClearGBufferPass::ClearGBufferPass(const std::string &passName)
: ExecutablePass(passName)
, pGBuffer0(this, "GBuffer0")
, pGBuffer1(this, "GBuffer1")
, pGBuffer2(this, "GBuffer2")
{
}

void ClearGBufferPass::execute(dx12lib::DirectContextProxy pDirectCtx) {
	ExecutablePass::execute(pDirectCtx);
}

GBufferPass::GBufferPass(const std::string &passName)
: RenderQueuePass(passName, false, true)
, pGBuffer0(this, "GBuffer0")
, pGBuffer1(this, "GBuffer1")
, pGBuffer2(this, "GBuffer2")
{
}

void GBufferPass::execute(dx12lib::DirectContextProxy pDirectCtx) {
	RenderQueuePass::execute(pDirectCtx);
}

LightingPass::LightingPass(const std::string &passName)
: ComputablePass(passName)
, pGBuffer0(this, "GBuffer0")
, pGBuffer1(this, "GBuffer1")
, pGBuffer2(this, "GBuffer2")
, pDepthBuffer(this, "DepthBuffer")
, pLightingBuffer(this, "LightingBuffer")
{
}

std::shared_ptr<rgph::RenderGraph> createTBDRRenderGraph(TBDRApp *pApp) {
	std::shared_ptr<rgph::RenderGraph> pRenderGraph;

	auto pClearDsPass = std::make_shared<rgph::ClearDsPass>(ClearDsPass);
	auto pClearGBuffer = std::make_shared<ClearGBufferPass>(ClearGBuffer);
	auto pGBufferPass = std::make_shared<GBufferPass>(GBuffer);
	auto pLightingPass = std::make_shared<LightingPass>(Lighting);
	auto pLightingCopyToMainRt = std::make_shared<rgph::CopyPass>(LightingCopyToMainRt);
	auto pSkyBox = std::make_shared<d3d::SkyBoxPass>(SkyBox);
	auto pPostPass = std::make_shared<PostPass>(ColorGradingAndGammaCorrection);
	auto pMainRtCopyToBackRt = std::make_shared<rgph::CopyPass>(MainRtCopyToBackRt);
	auto pPresentPass = std::make_shared<rgph::PresentPass>(Present);

	/// Clear Depth Stencil Pass
	{
		auto getDepthStencil = [=]() {
			return pApp->getSwapChain()->getDepthStencil2D();
		};

		getDepthStencil >> pClearDsPass->pDepthStencil;
		pClearDsPass->pDepthStencil.preExecuteState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		pRenderGraph->addPass(pClearDsPass);
	}
	/// Clear GBuffer Pass
	{
		pApp->pGBuffer0 >> pClearGBuffer->pGBuffer0;
		pApp->pGBuffer1 >> pClearGBuffer->pGBuffer1;
		pApp->pGBuffer2 >> pClearGBuffer->pGBuffer2;
		pClearGBuffer->pGBuffer0.preExecuteState = D3D12_RESOURCE_STATE_RENDER_TARGET;
		pClearGBuffer->pGBuffer1.preExecuteState = D3D12_RESOURCE_STATE_RENDER_TARGET;
		pClearGBuffer->pGBuffer1.preExecuteState = D3D12_RESOURCE_STATE_RENDER_TARGET;
		pRenderGraph->addPass(pClearGBuffer);
	}
	/// GBuffer Pass
	{
		pClearGBuffer->pGBuffer0 >> pGBufferPass->pGBuffer0;
		pClearGBuffer->pGBuffer1 >> pGBufferPass->pGBuffer1;
		pClearGBuffer->pGBuffer2 >> pGBufferPass->pGBuffer2;
		pClearDsPass->pDepthStencil >> pGBufferPass->pDepthStencil;
		pRenderGraph->addPass(pGBufferPass);
	}
	/// Lighting Pass
	{
		pClearGBuffer->pGBuffer0.preExecuteState = D3D12_RESOURCE_STATE_GENERIC_READ;
		pGBufferPass->pGBuffer0 >> pLightingPass->pGBuffer0;

		pClearGBuffer->pGBuffer1.preExecuteState = D3D12_RESOURCE_STATE_GENERIC_READ;
		pGBufferPass->pGBuffer1 >> pLightingPass->pGBuffer1;

		pClearGBuffer->pGBuffer2.preExecuteState = D3D12_RESOURCE_STATE_GENERIC_READ;
		pGBufferPass->pGBuffer2 >> pLightingPass->pGBuffer2;

		pLightingPass->pDepthBuffer.preExecuteState = D3D12_RESOURCE_STATE_DEPTH_READ;
		pGBufferPass->pDepthStencil >> pLightingPass->pDepthBuffer;

		pLightingPass->pLightingBuffer.preExecuteState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		pApp->pLightingBuffer >> pLightingPass->pLightingBuffer;
		pRenderGraph->addPass(pLightingPass);
	}
	/// copy lighting to main rt
	{
		auto getRenderTarget = [=]() {
			return pApp->getSwapChain()->getRenderTarget2D();
		};

		pLightingPass->pLightingBuffer >> pLightingCopyToMainRt->pSrcResource;
		getRenderTarget >> pLightingCopyToMainRt->pDstResource;
		pRenderGraph->addPass(pLightingCopyToMainRt);
	}
	/// SkyBox Pass
	{
		pSkyBox->pRenderTarget.preExecuteState = D3D12_RESOURCE_STATE_RENDER_TARGET;
		pLightingCopyToMainRt->pDstResource >> pSkyBox->pRenderTarget;
		pSkyBox->pDepthStencil.preExecuteState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		pGBufferPass->pDepthStencil >> pSkyBox->pDepthStencil;
		pRenderGraph->addPass(pGBufferPass);
	}
	/// ColorGrading & GammaCorrection
	{
		pSkyBox->pRenderTarget >> pPostPass->pBackBuffer;

	}
}

}
