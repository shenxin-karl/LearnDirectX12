#include "CommandList.h"
#include "FrameResourceQueue.h"
#include "Device.h"

namespace dx12lib {

FrameResourceItem::FrameResourceItem(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE cmdListType)
: _fence(0), _pDevice(pDevice), _cmdListType(cmdListType)
{
	ID3D12Device *pD3DDevice = pDevice.lock()->getD3DDevice();
	ThrowIfFailed(pD3DDevice->CreateCommandAllocator(
		cmdListType,
		IID_PPV_ARGS(&_pCmdListAlloc)
	));
}

uint64 FrameResourceItem::getFence() const noexcept {
	return _fence;
}

void FrameResourceItem::setFence(uint64 fence) noexcept {
	_fence = fence;
}

CommandListProxy FrameResourceItem::createCommandListProxy() {
	std::shared_ptr<CommandList> pCmdList;
	if (!_availableCmdList.tryPop(pCmdList)) {
		pCmdList = std::make_shared<CommandList>(weak_from_this());
		_cmdListPool.push(pCmdList);
	}
	pCmdList->getD3DCommandList()->Reset(_pCmdListAlloc.Get(), nullptr);
	return CommandListProxy(pCmdList, weak_from_this());
}

void FrameResourceItem::releaseCommandList(std::shared_ptr<CommandList> pCommandList) {
	_availableCmdList.push(pCommandList);
}

std::weak_ptr<Device> FrameResourceItem::getDevice() const noexcept {
	return _pDevice;
}

D3D12_COMMAND_LIST_TYPE FrameResourceItem::getCommandListType() const noexcept {
	return _cmdListType;
}

WRL::ComPtr<ID3D12CommandAllocator> FrameResourceItem::getCommandListAllocator() const noexcept {
	return _pCmdListAlloc;
}

FrameResourceQueue::FrameResourceQueue(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE cmdListType)
: _currentFrameResourceIndex(0) 
{
	_frameResourceItemCount = cmdListType == D3D12_COMMAND_LIST_TYPE_DIRECT ? kFrameResourceCount : 1;
	for (uint32 i = 0; i < _frameResourceItemCount; ++i)
		_frameResourceQueue[i] = std::make_unique<FrameResourceItem>(pDevice, cmdListType);
}

CommandListProxy FrameResourceQueue::createCommandListProxy() {
	return _frameResourceQueue[_currentFrameResourceIndex]->createCommandListProxy();
}

uint32 FrameResourceQueue::getMaxFrameResourceCount() const noexcept {
	return _frameResourceItemCount;
}

void FrameResourceQueue::newFrame(uint64 fence) {
	_frameResourceQueue[_currentFrameResourceIndex]->setFence(fence);
	_currentFrameResourceIndex = (_currentFrameResourceIndex + 1) % _frameResourceItemCount;
}

}