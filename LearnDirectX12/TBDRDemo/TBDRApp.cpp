#include "TBDRApp.h"
#include "D3D/Model/MeshModel/MeshModel.h"
#include "Dx12lib/Texture/Texture.h"

TBDRApp::TBDRApp() {
}

TBDRApp::~TBDRApp() {
}


void test(dx12lib::DirectContextProxy pDirectCtx) {
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	clearValue.Color[0] = 1.f;
	clearValue.Color[1] = 1.f;
	clearValue.Color[2] = 1.f;
	clearValue.Color[3] = 1.f;

	auto desc = dx12lib::Texture::make2D(
		DXGI_FORMAT_R8G8B8A8_UNORM, 256, 256, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	);
	auto pRenderTarget = pDirectCtx->createTexture(desc, &clearValue);

	assert(pRenderTarget->get2dSRV().valid());
	assert(pRenderTarget->get2dRTV().valid());
	assert(!pRenderTarget->get2dUAV().valid());
	assert(!pRenderTarget->get2dDSV().valid());
	assert(!pRenderTarget->getArrayRTV().valid());
	assert(pRenderTarget->get2dRTV().valid());

	desc = dx12lib::Texture::make2D(DXGI_FORMAT_R16G16B16A16_FLOAT, 512, 512,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
	clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	auto pBackBuffer = pDirectCtx->createTexture(desc, &clearValue);
	assert(pBackBuffer->get2dRTV().valid());
	assert(pBackBuffer->get2dUAV().valid());


	desc = dx12lib::Texture::makeArray(DXGI_FORMAT_D16_UNORM,
		1024, 1024, 4,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);
	clearValue.Format = DXGI_FORMAT_D16_UNORM;
	clearValue.DepthStencil.Depth = 1.f;
	clearValue.DepthStencil.Stencil = 0;
	auto pShadowMapArray = pDirectCtx->createTexture(desc, &clearValue);
	assert(pShadowMapArray->getArrayDSV().valid());
	assert(pShadowMapArray->getArraySRV().valid());
	assert(pShadowMapArray->getPlaneSRV(1).valid());
	assert(pShadowMapArray->getPlaneDSV(3).valid());


}

void TBDRApp::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
	test(pDirectCtx);
	auto pAlTree = std::make_shared<d3d::ALTree>("resources/SponzaPBR/Sponza.gltf");
	_pMeshModel = std::make_unique<d3d::MeshModel>(*pDirectCtx, pAlTree);
}

void TBDRApp::onDestroy() {
}

void TBDRApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	BaseApp::onBeginTick(pGameTimer);
}

void TBDRApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	BaseApp::onTick(pGameTimer);
}

void TBDRApp::onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	BaseApp::onEndTick(pGameTimer);
}

void TBDRApp::onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) {
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	_pCamera->setAspect(aspect);
}

auto TBDRApp::getSwapChain() const -> std::shared_ptr<dx12lib::SwapChain> {
	return _pSwapChain;
}
