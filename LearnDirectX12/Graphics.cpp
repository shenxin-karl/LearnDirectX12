#include <comdef.h>
#include <dxgi1_4.h>
#include "Graphics.h"
#include "D3Dx12.h"


Graphics::Graphics() {
	// enabling debugging
#if defined(DEBUG) || defined(_DEBUG)
	{
		WRL::ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	// step1
	WRL::ComPtr<IDXGIFactory4> dxgiFactory;
	CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory_)));
	HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice_));
	if (FAILED(hr)) {
		WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(), 
			D3D_FEATURE_LEVEL_11_0, 
			IID_PPV_ARGS(&d3dDevice_)
		));
	}

	// step2
	ThrowIfFailed(d3dDevice_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));
	rtvDescriptorSize_ = d3dDevice_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	dsvDescriptorSize_ = d3dDevice_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	cbvUavDescriptorSize_ = d3dDevice_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// step3
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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
