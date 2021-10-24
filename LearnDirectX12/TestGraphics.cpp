#include <DirectXMath.h>
#include <DirectXColors.h>
#include "TestGraphics.h"
#include "D3DApp.h"
#include "window.h"

void TestGraphics::initialize() {
	Base::initialize();
}

void TestGraphics::tick(GameTimer &dt) {
	if (D3DApp::instance()->getWindow()->isPause())
		return;
	Base::tick(dt);
	draw();
}

void TestGraphics::update() {
	Base::update();
}

void TestGraphics::draw() {
	ThrowIfFailed(directCmdListAlloc_->Reset());
	ThrowIfFailed(commandList_->Reset(
		directCmdListAlloc_.Get(), nullptr
	));

	auto transitionDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		currentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	commandList_->ResourceBarrier(1, &transitionDesc); 

	commandList_->RSSetViewports(1, &screenViewport_);
	commandList_->RSSetScissorRects(1, &scissiorRect_);

	// clear back target view and depth and stencil buffer
	commandList_->ClearRenderTargetView(
		currentBackBufferView(),
		DirectX::Colors::LightSteelBlue,
		0,
		nullptr
	);
	commandList_->ClearDepthStencilView(
		depthStencilView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f, 0, 0, nullptr
	);

	auto currentBackBufferViewObj = currentBackBufferView();
	auto depthStencilViewObj = depthStencilView();
	commandList_->OMSetRenderTargets(1, &currentBackBufferViewObj, true, &depthStencilViewObj);

	// transition resource state
	transitionDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		currentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	commandList_->ResourceBarrier(1, &transitionDesc);

	// close command list
	ThrowIfFailed(commandList_->Close());

	// the commands to be executed added to command list
	ID3D12CommandList *cmdLists[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(static_cast<UINT>(std::size(cmdLists)), cmdLists);

	// swap buffers
	ThrowIfFailed(swapChain_->Present(0, 0));
	currBackBuffer_ = (currBackBuffer_ + 1) % kSwapChainCount; 

	// wait GPU render
	flushCommandQueue();
}

void TestGraphics::onResize() {
	Base::onResize();
}

void TestGraphics::handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	Base::handleMsg(hwnd, msg, wParam, lParam);
}
