#include <comdef.h>
#include <dxgi1_4.h>
#include "Graphics.h"
#include "d3dulti.h"
#include "D3DApp.h"
#include "Window.h"
#include "GameTimer.h"

void Graphics::initialize() {
	// enabling debugging
#if defined(DEBUG) || defined(_DEBUG)
	{
		WRL::ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	// step1 try to create hardward device
	WRL::ComPtr<IDXGIFactory4> dxgiFactory;
	CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory_)));
	HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice_));
	if (FAILED(hr)) {			// fallback to wapp device
		WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&d3dDevice_)
		));
	}

	// step2 create GPU Fence
	ThrowIfFailed(d3dDevice_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));
	rtvDescriptorSize_ = d3dDevice_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	dsvDescriptorSize_ = d3dDevice_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	cbvUavDescriptorSize_ = d3dDevice_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// step3 check 4x msaa
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = backBufferFormat_;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(d3dDevice_->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)
	));
	msaaQualityLevel_ = msQualityLevels.NumQualityLevels;
	assert(msaaQualityLevel_ > 0 && "Unexpected MSAA quality level");

	// step4
	createCommandObjects();
	// step5
	createSwapChain();
	// step6
	createRtvAndDsvDescriptorHeaps();

	// first initialize on resize
	onResize();
}

void Graphics::createCommandObjects() {
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(d3dDevice_->CreateCommandQueue(		// create Command Queue
		&queueDesc, IID_PPV_ARGS(&commandQueue_)
	));
	ThrowIfFailed(d3dDevice_->CreateCommandAllocator(	// create Command Allocator
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&directCmdListAlloc_)
	));
	ThrowIfFailed(d3dDevice_->CreateCommandList(		// create Command List
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		directCmdListAlloc_.Get(),
		nullptr,										// Not in use now
		IID_PPV_ARGS(&commandList_)
	));
	// 在第一次使用的时候需要重置, 所以现在需要关闭它
	commandList_->Close();								
}

void Graphics::createSwapChain() {
	swapChain_.Reset();
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = D3DApp::instance()->getWindow()->getWidth();
	sd.BufferDesc.Height = D3DApp::instance()->getWindow()->getHeight();
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = backBufferFormat_;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = msaaState_ ? 4 : 1;
	sd.SampleDesc.Quality = msaaState_ ? (msaaQualityLevel_-1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = kSwapChainCount;
	sd.OutputWindow = D3DApp::instance()->getWindow()->getHWND();
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	ThrowIfFailed(dxgiFactory_->CreateSwapChain(
		commandQueue_.Get(),
		&sd,
		swapChain_.GetAddressOf()
	));
}

void Graphics::flushCommandQueue() {
	++currentFence_;	
	ThrowIfFailed(commandQueue_->Signal(fence_.Get(), currentFence_));
	if (fence_->GetCompletedValue() < currentFence_) {
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(fence_->SetEventOnCompletion(currentFence_, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void Graphics::createRtvAndDsvDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = kSwapChainCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(d3dDevice_->CreateDescriptorHeap(
		&rtvHeapDesc,
		IID_PPV_ARGS(&rtvHeap_)
	));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(d3dDevice_->CreateDescriptorHeap(
		&dsvHeapDesc,
		IID_PPV_ARGS(&dsvHeap)
	));
}
void Graphics::calculateFrameStats() {
	static int frameCnt = 0;
	static float timeElapsed = 0.f;	
	++frameCnt;
	auto timerPtr = D3DApp::instance()->getGameTimer();
	if ((timerPtr->totalTime() - timeElapsed) >= 1.0f) {
		float fps = static_cast<float>(frameCnt);
		float mspf = 1000.f / fps;

		std::string title = D3DApp::instance()->getWindow()->getTitle();
		title = std::format("{} fps: {} mspf: {}");
		D3DApp::instance()->getWindow()->setShowTitle(title);

		frameCnt = 0;
		timeElapsed += 1.f;
	}
}

void Graphics::set4xMsaaState(bool val) {
	if (val != msaaState_) {
		createSwapChain();
		onResize();
	}
}

ID3D12Resource *Graphics::currentBackBuffer() const {
	return swapChainBuffer_[currBackBuffer_].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Graphics::currentBackBufferView() const {
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		rtvHeap_->GetCPUDescriptorHandleForHeapStart(),
		currBackBuffer_,
		rtvDescriptorSize_
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE Graphics::depthStencilView() const {
	return dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void Graphics::onResize() {
	assert(d3dDevice_ != nullptr);
	assert(swapChain_ != nullptr);
	assert(directCmdListAlloc_ != nullptr);

	flushCommandQueue();

	ThrowIfFailed(commandList_->Reset(directCmdListAlloc_.Get(), nullptr));

	// release the previous resources 
	for (int i = 0; i < kSwapChainCount; ++i)
		swapChainBuffer_[i].Reset();
	depthStencilBuffer_.Reset();


	// resize the swap chain
	ThrowIfFailed(swapChain_->ResizeBuffers(
		kSwapChainCount,
		D3DApp::instance()->getWindow()->getWidth(), D3DApp::instance()->getWindow()->getHeight(),
		backBufferFormat_,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	));

	currBackBuffer_ = 0;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(rtvHeap_->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < kSwapChainCount; ++i) {
		ThrowIfFailed(swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainBuffer_[i])));
		d3dDevice_->CreateRenderTargetView(swapChainBuffer_[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, rtvDescriptorSize_);
	}

	int clientWidth = D3DApp::instance()->getWindow()->getWidth();
	int clientHeight = D3DApp::instance()->getWindow()->getHeight();
	// create the depth/stencil buffer and view
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = clientWidth;
	depthStencilDesc.Height = clientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = msaaState_ ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = msaaState_ ? (msaaQualityLevel_-1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = depthStencilFormat_;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(d3dDevice_->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(&depthStencilBuffer_)
	));

	// create descripotr to mip level 0 of entire resource using the format of resource
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = depthStencilFormat_;
	dsvDesc.Texture2D.MipSlice = 0;
	d3dDevice_->CreateDepthStencilView(depthStencilBuffer_.Get(), &dsvDesc, depthStencilView());

	// execute the resize commands
	ThrowIfFailed(commandList_->Close());
	ID3D12CommandList *cmdLists[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(static_cast<UINT>(std::size(cmdLists)), cmdLists);

	// wait resize is complete
	flushCommandQueue();

	// update thie viewport transform to cover the client area
	screenViewport_.TopLeftX = 0;
	screenViewport_.TopLeftY = 0;
	screenViewport_.Width = static_cast<float>(clientWidth);
	screenViewport_.Height = static_cast<float>(clientHeight);
	screenViewport_.MinDepth = 0.f;
	screenViewport_.MaxDepth = 1.f;

	scissiorRect_ = { 0, 0, clientWidth, clientHeight };
}

Graphics::~Graphics() {
	if (d3dDevice_ != nullptr)
		flushCommandQueue();
}

GraphicsException::GraphicsException(const char *file, int line, HRESULT hr)
: ExceptionBase(file, line), hr_(hr) {
}

const char *GraphicsException::what() const noexcept {
	ExceptionBase::what();
	whatBuffer_ += getErrorString();
	return whatBuffer_.c_str();
}

const char *GraphicsException::getType() const noexcept {
	return "GraphicsException";
}

std::string GraphicsException::getErrorString() const {
	_com_error err(hr_);
	std::string msg = err.ErrorMessage();
	return msg;
}

