#include "CommandList.h"
#include "Device.h"
#include "FrameResourceQueue.h"

namespace dx12lib {

ID3D12GraphicsCommandList * CommandList::getD3DCommandList() const noexcept {
	return _pCommandList.Get();
}

HRESULT CommandList::close() {
	return _pCommandList->Close();
}

CommandList::CommandList(std::weak_ptr<FrameResourceItem> pFrameResourceItem) {
	auto pSharedFrameResourceItem = pFrameResourceItem.lock();
	auto pDevice = pSharedFrameResourceItem->getDevice();
	auto pd3d12Device = pDevice.lock()->getD3DDevice();
	ThrowIfFailed(pd3d12Device->CreateCommandList(
		0,
		pSharedFrameResourceItem->getCommandListType(),
		pSharedFrameResourceItem->getCommandListAllocator().Get(),
		nullptr,
		IID_PPV_ARGS(&_pCommandList)
	));
}

}