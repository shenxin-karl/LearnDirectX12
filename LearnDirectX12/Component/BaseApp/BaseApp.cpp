#include "BaseApp.h"
#include "D3D/d3dulti.h"
#include "InputSystem/InputSystem.h"
#include "InputSystem/Window.h"

bool com::BaseApp::initialize() {
	pInputSystem_ = std::make_unique<InputSystem>(title_, width_, height_);
	if (!initializeD3D())
		return false;
	onResize(width_, height_);
	return true;
}

void com::BaseApp::beginTick(std::shared_ptr<GameTimer> pGameTimer) {
	pInputSystem_->beginTick(pGameTimer);
}

void com::BaseApp::tick(std::shared_ptr<GameTimer> pGameTimer) {
	pInputSystem_->tick(pGameTimer);
}

void com::BaseApp::endTick(std::shared_ptr<GameTimer> pGameTimer) {
	pInputSystem_->tick(pGameTimer);
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
	rtvHeapDesc.NodeMask = 0;
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
		pDevice_.Get(),
		&sd,
		&pSwapChain_
	));
}

UINT com::BaseApp::getSampleCount() const {
	return msaaState_ ? 4 : 1;
}

UINT com::BaseApp::getSampleQuality() const {
	return msaaState_ ? (msaaQuality_-1) : 0;
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
	return true;
}
