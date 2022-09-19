#include "ShadowRgph.h"
#include "ShadowApp.h"
#include "D3D/Shadow/CSMShadowPass.h"
#include "D3D/Sky/SkyBoxPass.h"
#include "D3D/Tool/FirstPersonCamera.h"
#include "Dx12lib/Device/SwapChain.h"
#include "RenderGraph/Pass/ClearPass.hpp"
#include "RenderGraph/Pass/PresentPass.hpp"
#include "Dx12lib/Resource/IResource.h"

ShadowPass::ShadowPass(const std::string &passName) : RenderQueuePass(passName, false, true) {
}

OpaquePass::OpaquePass(const std::string &passName)
	: RenderQueuePass(passName)
	, pShadowMapArray(this, "ShadowMapArray")
{
}

std::shared_ptr<rgph::RenderGraph> createShadowRenderGraph(ShadowApp *pApp, dx12lib::DirectContextProxy pDirectCtx) {
	auto pClearPass = std::make_shared<rgph::ClearPass>(ShadowRgph::ClearRTAndDSPass);
	auto pClearCSMShadowMap = std::make_shared<d3d::ClearCSMShadowMapPass>(ShadowRgph::ClearCSMShadowMapPass);
	auto pOpaquePass = std::make_shared<OpaquePass>(ShadowRgph::OpaquePass);
	auto pSkyBoxPass = std::make_shared<d3d::SkyBoxPass>(ShadowRgph::SkyBoxPass);
	auto pPresentPass = std::make_shared<rgph::PresentPass>(ShadowRgph::PresentPass);
	auto pCSMShadowPass = std::make_shared<d3d::CSMShadowPass>(ShadowRgph::ShadowPass);

	pCSMShadowPass->finalize(pDirectCtx);

	std::shared_ptr<rgph::RenderGraph> pRenderGraph = std::make_shared<rgph::RenderGraph>();
	{ // clear RenderTarget and DepthStencil Pass
		auto getRenderTarget = [=]() {
			return pApp->getSwapChain()->getRenderTarget2D();
		};
		auto getDepthStencil = [=]() {
			return pApp->getSwapChain()->getDepthStencil2D();
		};

		getRenderTarget >> pClearPass->pRenderTarget;
		getDepthStencil >> pClearPass->pDepthStencil;
		pRenderGraph->addPass(pClearPass);
	}
	{ // clear Shadow Map
		pClearCSMShadowMap->pShadowMapArray.preExecuteState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		pCSMShadowPass->getShadowMapArray() >> pClearCSMShadowMap->pShadowMapArray;
		pRenderGraph->addPass(pClearCSMShadowMap);
	}
	{ // shadow pass
		pClearCSMShadowMap->pShadowMapArray >> pCSMShadowPass->pShadowMapArray;
		pRenderGraph->addPass(pCSMShadowPass);
	}
	{ // opaque pass
		pOpaquePass->setPassCBuffer(pApp->getPassCb());
		pOpaquePass->pShadowMapArray.preExecuteState = D3D12_RESOURCE_STATE_DEPTH_READ;
		pClearPass->pRenderTarget >> pOpaquePass->pRenderTarget;
		pClearPass->pDepthStencil >> pOpaquePass->pDepthStencil;
		pCSMShadowPass->pShadowMapArray >> pOpaquePass->pShadowMapArray;
		pRenderGraph->addPass(pOpaquePass);
	}
	{ // SkyBoxPass
		pSkyBoxPass->renderTargetFormat = pApp->getSwapChain()->getRenderTargetFormat();
		pSkyBoxPass->depthStencilFormat = pApp->getSwapChain()->getDepthStencilFormat();
		pSkyBoxPass->pEnvMap = pApp->getEnvMap();
		pSkyBoxPass->pCamera = pApp->getCamera().get();
		pOpaquePass->pRenderTarget >> pSkyBoxPass->pRenderTarget;
		pOpaquePass->pDepthStencil >> pSkyBoxPass->pDepthStencil;
		pRenderGraph->addPass(pSkyBoxPass);
	}
	{ // Present Pass
		pSkyBoxPass->pRenderTarget >> pPresentPass->pRenderTarget;
		pRenderGraph->addPass(pPresentPass);
	}
	pRenderGraph->finalize();
	return pRenderGraph;
}
