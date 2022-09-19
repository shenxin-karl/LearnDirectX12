#include "TBDRRgph.h"
#include "D3D/Sky/SkyBoxPass.h"

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
	auto pClearGBuffer = std::make_shared<ClearGBufferPass>(ClearGBuffer);
	auto pGBufferPass = std::make_shared<GBufferPass>(GBuffer);
	auto pLightingPass = std::make_shared<LightingPass>(Lighting);
	auto pSkyBox = std::make_shared<d3d::SkyBoxPass>(SkyBox);
	auto pPostPass = std::make_shared<PostPass>();
}

}
