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
	ID3D12GraphicsCommandList *getD3DCommandList() const noexcept;
	HRESULT close();
private:
	friend class FrameResourceItem;
	CommandList(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE cmdListType);
private:
	D3D12_COMMAND_LIST_TYPE                 _cmdListType;
	std::weak_ptr<Device>                   _pDevice;
	WRL::ComPtr<ID3D12GraphicsCommandList>  _pCommandList;
};

}