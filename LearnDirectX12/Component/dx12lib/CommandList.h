#pragma once
#include "dx12libStd.h"
#include <memory>

namespace dx12lib {

class IndexBuffer;
class VertexBuffer;
class ConstantBuffer;
class Device;
class FrameResourceItem;
class CommandListProxy;

class CommandList {
public:
	CommandList(std::weak_ptr<FrameResourceItem> pFrameResourceItem);
	ID3D12GraphicsCommandList *getD3DCommandList() const noexcept;
	HRESULT close();
private:
	D3D12_COMMAND_LIST_TYPE                 _cmdListType;
	std::weak_ptr<Device>                   _pDevice;
	WRL::ComPtr<ID3D12GraphicsCommandList>  _pCommandList;
};

}