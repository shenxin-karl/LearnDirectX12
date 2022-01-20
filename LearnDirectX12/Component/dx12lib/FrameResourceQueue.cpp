#include "FrameResourceQueue.h"
#include "CommandList.h"
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

std::shared_ptr<CommandList> FrameResourceItem::createCommandList() {
	std::shared_ptr<CommandList> pCmdList;
	if (!_availableCmdList.empty()) {
		pCmdList = _availableCmdList.back();
		_availableCmdList.pop_back();
	} else {
		pCmdList = std::make_shared<CommandList>(_pDevice, _cmdListType);
	}
	pCmdList->getD3DCommandList()->Reset(_pCmdListAlloc.Get(), nullptr);
	return pCmdList;
}

void FrameResourceItem::reset() {
	_availableCmdList = _cmdListPool;
}

FrameResourceQueue::FrameResourceQueue(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE cmdListType)
: _currentFrameResourceIndex(0) 
{
	_frameResourceItemCount = cmdListType == D3D12_COMMAND_LIST_TYPE_DIRECT ? kFrameResourceCount : 1;
	for (uint32 i = 0; i < _frameResourceItemCount; ++i)
		_frameResourceQueue[i] = std::make_unique<FrameResourceItem>(pDevice, cmdListType);
}

std::shared_ptr<CommandList> FrameResourceQueue::createCommandList() {
	return _frameResourceQueue[_currentFrameResourceIndex]->createCommandList();
}

uint32 FrameResourceQueue::getMaxFrameResourceCount() const noexcept {
	return _frameResourceItemCount;
}

void FrameResourceQueue::newFrame(uint64 fence) {
	_frameResourceQueue[_currentFrameResourceIndex]->setFence(fence);
	_currentFrameResourceIndex = (_currentFrameResourceIndex + 1) % _frameResourceItemCount;
	_frameResourceQueue[_currentFrameResourceIndex]->reset();
}

}