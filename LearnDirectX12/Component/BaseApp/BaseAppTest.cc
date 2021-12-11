#include <iostream>
#include <DirectXColors.h>
#include "BaseApp.h"
#include "D3D/d3dutil.h"
#include "Exception/ExceptionBase.h"
#include "GameTimer/GameTimer.h"


class TestApp : public com::BaseApp {
public:
	TestApp();
	virtual void tick(std::shared_ptr<com::GameTimer> pGameTimer) override;
};

TestApp::TestApp() {

}

void TestApp::tick(std::shared_ptr<com::GameTimer> pGameTimer) {
	namespace DX = DirectX;
	ThrowIfFailed(pCommandAlloc_->Reset());
	ThrowIfFailed(pCommandList_->Reset(pCommandAlloc_.Get(), nullptr));
	pCommandList_->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
		getCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	)));

	pCommandList_->RSSetViewports(1, &screenViewport_);
	pCommandList_->RSSetScissorRects(1, &scissorRect_);

	DX::XMVECTORF32 color = {
		0.5f,
		(std::sin(pGameTimer->getTotalTime()) + 1.f) * 0.5f,
		(std::cos(pGameTimer->getTotalTime()) + 1.f) * 0.5f,
		1.f
	};
	pCommandList_->ClearRenderTargetView(getCurrentBackBufferView(), color, 0, nullptr);
	pCommandList_->ClearDepthStencilView(getDepthStencilBufferView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr
	);

	pCommandList_->OMSetRenderTargets(
		1, RVPtr(getCurrentBackBufferView()), 
		true, RVPtr(getDepthStencilBufferView())
	);

	pCommandList_->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
		getCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	)));

	ThrowIfFailed(pCommandList_->Close());
	ID3D12CommandList *cmdLists[] = { pCommandList_.Get() };
	pCommandQueue_->ExecuteCommandLists(1, cmdLists);

	ThrowIfFailed(pSwapChain_->Present(0, 0));
	currentBackBufferIndex_ = (currentBackBufferIndex_ + 1) % kSwapChainCount;

	flushCommandQueue();
}

int main() {
	std::shared_ptr<com::GameTimer> pGameTimer = std::make_shared<com::GameTimer>();
	try
	{
		TestApp app;
		app.initialize();
		while (!app.shouldClose()) {
			app.beginTick(pGameTimer);
			app.tick(pGameTimer);
			app.endTick(pGameTimer);
		}
	} catch (const com::ExceptionBase &e) {
		std::cerr << e.what() << std::endl;
		OutputDebugStringA(e.what());
	}
	return 0;
}