#pragma once
#include "dx12libCommon.h"
#include <memory>

namespace dx12lib {

class IndexBuffer;
class VertexBuffer;
class ConstantBuffer;
class Device;

class CommandList {
public:
	CommandList(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE cmdListType);
	ID3D12GraphicsCommandList *getD3DCommandList() const noexcept;
	std::shared_ptr<VertexBuffer> createVertexBuffer(
		const void *pData,
		uint32 sizeInByte,
		uint32 vertexStride
	) const;
	std::shared_ptr<IndexBuffer> createIndexBuffer(
		const void *pData,
		uint32 sizeInByte,
		DXGI_FORMAT indexFormat
	) const;
	std::shared_ptr<ConstantBuffer> createConstantBuffer(
		const void *pData,
		uint32 sizeInByte
	) const;

	HRESULT close();
private:
	D3D12_COMMAND_LIST_TYPE _cmdListType;
	std::weak_ptr<Device> _pDevice;
	WRL::ComPtr<ID3D12GraphicsCommandList>  _pCommandList;
};

}