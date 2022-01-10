#pragma once
#include "dx12libCommon.h"
#include <memory>


namespace dx12lib {

class Adapter;
class VertexBuffer;
class IndexBuffer;
class SwapChain;

class Device {
public:
	Device(std::shared_ptr<Adapter> pAdapter);
	~Device() = default;
	std::shared_ptr<VertexBuffer> createVertexBuffer() const;
	std::shared_ptr<IndexBuffer> createIndexBuffer() const;
	std::shared_ptr<SwapChain> createSwapChain() const;
private:
	WRL::ComPtr<ID3D12Device>  _pDevice;
};

}