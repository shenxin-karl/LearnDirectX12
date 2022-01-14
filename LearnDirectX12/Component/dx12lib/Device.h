#pragma once
#include "dx12libCommon.h"
#include <memory>


namespace dx12lib {

class Adapter;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class SwapChain;
class CommandQueue;

class Device : public std::enable_shared_from_this<Device> {
public:
	Device(std::shared_ptr<Adapter> pAdapter);
	~Device() = default;

	std::shared_ptr<SwapChain> createSwapChain(
		HWND hwnd,
		DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R10G10B10A2_UNORM,
		DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT
	) const;

	UINT getSampleCount() const;
	UINT getSampleQuality() const;
public:
	std::shared_ptr<Adapter> getAdapter() const;
	std::shared_ptr<CommandQueue> getCommandQueue(CommandQueueType type) const;
private:
	WRL::ComPtr<ID3D12Device>     _pDevice;
	std::shared_ptr<Adapter>      _pAdapter;
	std::shared_ptr<CommandQueue> _pCommandQueueList[kComandQueueTypeCount];
	UINT _4xMsaaQuality = 0;
	UINT _4xMsaaState = false;
};

}