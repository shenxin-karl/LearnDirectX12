#include "BaseApp.h"
#include "D3D/d3dulti.h"
#include "InputSystem/Window.h"
#include <DirectXColors.h>

namespace com {

bool com::BaseApp::initialize() {
	pInputSystem_ = std::make_unique<InputSystem>(title_, width_, height_);
	if (!initializeD3D())
		return false;

	onResize(width_, height_);
	pInputSystem_->window->setResizeCallback([this](int width, int height) {
		onResize(width, height);
	});
	return true;
}

void com::BaseApp::beginTick(std::shared_ptr<GameTimer> pGameTimer) {
	pInputSystem_->beginTick(pGameTimer);
}

void com::BaseApp::tick(std::shared_ptr<GameTimer> pGameTimer) {
	pInputSystem_->tick(pGameTimer);
}

void com::BaseApp::endTick(std::shared_ptr<GameTimer> pGameTimer) {
	pInputSystem_->endTick(pGameTimer);
}

void com::BaseApp::onResize(int width, int height) {
	width_ = width;
	height_ = height;
	flushCommandQueue();
	ThrowIfFailed(pCommandList_->Reset(pCommandAlloc_.Get(), nullptr));

	for (size_t i = 0; i < kSwapChainCount; ++i)
		pSwapChainBuffer_[i].Reset();
	pDepthStencilBuffer_.Reset();

	ThrowIfFailed(pSwapChain_->ResizeBuffers(
		kSwapChainCount,
		width,
		height,
		backBufferFormat_,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	));

	currentBackBufferIndex_ = 0;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(pRtvHeap_->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < kSwapChainCount; ++i) {
		ThrowIfFailed(pSwapChain_->GetBuffer(i, IID_PPV_ARGS(&pSwapChainBuffer_[i])));
		pDevice_->CreateRenderTargetView(pSwapChainBuffer_[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, rtvDescriptorSize_);
	}

	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = getSampleCount();
	depthStencilDesc.SampleDesc.Quality = getSampleQuality();
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = depthStencilFormat_;
	optClear.DepthStencil.Depth = 1.f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(pDevice_->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(&pDepthStencilBuffer_)
	));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = depthStencilFormat_;
	dsvDesc.Texture2D.MipSlice = 0;
	pDevice_->CreateDepthStencilView(pDepthStencilBuffer_.Get(), &dsvDesc, getDepthStencilBufferView());

	pCommandList_->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
		pDepthStencilBuffer_.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
	)));

	ThrowIfFailed(pCommandList_->Close());
	ID3D12CommandList *cmdLists[] = { pCommandList_.Get() };
	pCommandQueue_->ExecuteCommandLists(1, cmdLists);
	flushCommandQueue();

	screenViewport_.Width = static_cast<float>(width);
	screenViewport_.Height = static_cast<float>(height);
	screenViewport_.MinDepth = 0.f;
	screenViewport_.MaxDepth = 1.f;
	screenViewport_.TopLeftX = 0;
	screenViewport_.TopLeftY = 0;

	scissorRect_ = { 0, 0, width, height };
}

com::BaseApp::~BaseApp() {
	if (pDevice_ != nullptr)
		flushCommandQueue();
}

void com::BaseApp::createCommandObjects() {
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(pDevice_->CreateCommandQueue(
		&queueDesc,
		IID_PPV_ARGS(&pCommandQueue_
	)));
	ThrowIfFailed(pDevice_->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&pCommandAlloc_)
	));
	ThrowIfFailed(pDevice_->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		pCommandAlloc_.Get(),
		nullptr,
		IID_PPV_ARGS(&pCommandList_)
	));
	pCommandList_->Close();
}

void com::BaseApp::createRtvAndDsvDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = kSwapChainCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(pDevice_->CreateDescriptorHeap(
		&rtvHeapDesc,
		IID_PPV_ARGS(&pRtvHeap_)
	));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(pDevice_->CreateDescriptorHeap(
		&dsvHeapDesc,
		IID_PPV_ARGS(&pDsvHeap_)
	));
}

void com::BaseApp::creaetSwapChain() {
	pSwapChain_.Reset();
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = width_;
	sd.BufferDesc.Height = height_;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.Format = backBufferFormat_;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = getSampleCount();
	sd.SampleDesc.Quality = getSampleQuality();
	sd.OutputWindow = pInputSystem_->window->getHWND();
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = kSwapChainCount;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ThrowIfFailed(pDxgiFactory_->CreateSwapChain(
		pCommandQueue_.Get(),
		&sd,
		&pSwapChain_
	));
}

UINT com::BaseApp::getSampleCount() const {
	return msaaState_ ? 4 : 1;
}

UINT com::BaseApp::getSampleQuality() const {
	return msaaState_ ? (msaaQuality_ - 1) : 0;
}

void com::BaseApp::flushCommandQueue() {
	++currentFence_;
	ThrowIfFailed(pCommandQueue_->Signal(pFence_.Get(), currentFence_));
	if (pFence_->GetCompletedValue() < currentFence_) {
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(pFence_->SetEventOnCompletion(currentFence_, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE com::BaseApp::getCurrentBackBufferView() const {
	return CD3DX12_CPU_DESCRIPTOR_HANDLE{
		pRtvHeap_->GetCPUDescriptorHandleForHeapStart(),
		currentBackBufferIndex_,
		rtvDescriptorSize_
	};
}

D3D12_CPU_DESCRIPTOR_HANDLE com::BaseApp::getDepthStencilBufferView() const {
	return pDsvHeap_->GetCPUDescriptorHandleForHeapStart();
}

ID3D12Resource *com::BaseApp::getCurrentBackBuffer() {
	return pSwapChainBuffer_[currentBackBufferIndex_].Get();
}

ID3D12Resource *com::BaseApp::getDepthStencilBuffer() {
	return pDepthStencilBuffer_.Get();
}

bool com::BaseApp::initializeD3D() {
#if defined(DEBUG) || defined(_DEBUG)
	{
		WRL::ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&pDxgiFactory_)));
	HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice_));
	if (FAILED(hr)) {
		WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(pDxgiFactory_->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&pDevice_)
		));
	}

	ThrowIfFailed(pDevice_->CreateFence(
		0,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&pFence_)
	));
	rtvDescriptorSize_ = pDevice_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	dsvDescriptorSize_ = pDevice_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	cbvSrvUavDescriptorSize_ = pDevice_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = backBufferFormat_;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(pDevice_->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)
	));
	msaaQuality_ = msQualityLevels.NumQualityLevels;
	assert(msaaQuality_ > 0 && "Unexpected MSAA quality level");

	createCommandObjects();

	creaetSwapChain();

	createRtvAndDsvDescriptorHeaps();

	BaseApp::onResize(width_, height_);
	return true;
}

bool BaseApp::shouldClose() const {
	return pInputSystem_->shouldClose();
}

}