#include "CommandList.h"
#include "FrameResourceQueue.h"
#include "Device.h"
#include "MakeObejctTool.hpp"

namespace dx12lib {

FrameResourceItem::FrameResourceItem(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE cmdListType)
: _fence(0), _pDevice(pDevice), _cmdListType(cmdListType) {
}

uint64 FrameResourceItem::getFence() const noexcept {
	return _fence;
}

void FrameResourceItem::setFence(uint64 fence) noexcept {
	_fence = fence;
}

std::shared_ptr<CommandList> FrameResourceItem::createCommandList() {
	std::shared_ptr<CommandList> pCmdList;
	if (!_availableCmdList.tryPop(pCmdList)) {
		pCmdList = std::make_shared<dx12libTool::MakeCommandList>(weak_from_this());
		_cmdListPool.push(pCmdList);
		pCmdList->close();
	}
	pCmdList->reset();
	return pCmdList;
}

std::weak_ptr<Device> FrameResourceItem::getDevice() const noexcept {
	return _pDevice;
}

D3D12_COMMAND_LIST_TYPE FrameResourceItem::getCommandListType() const noexcept {
	return _cmdListType;
}

void FrameResourceItem::newFrame(uint64 fence) {
	_availableCmdList = _cmdListPool;
	setFence(fence);
}

FrameResourceQueue::FrameResourceQueue(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE cmdListType)
: _currentFrameResourceIndex(0) 
{
	_frameResourceItemCount = cmdListType == D3D12_COMMAND_LIST_TYPE_DIRECT ? kFrameResourceCount : 1;
	for (uint32 i = 0; i < _frameResourceItemCount; ++i)
		_frameResourceQueue[i] = std::make_shared<dx12libTool::MakeFrameResourceItem>(pDevice, cmdListType);
}

std::shared_ptr<CommandList> FrameResourceQueue::createCommandList() {
	return _frameResourceQueue[_currentFrameResourceIndex]->createCommandList();
}

uint32 FrameResourceQueue::getMaxFrameResourceCount() const noexcept {
	return _frameResourceItemCount;
}

std::atomic_uint32_t &FrameResourceQueue::getCurrentFrameResourceIndexRef() {
	return _currentFrameResourceIndex;
}

void FrameResourceQueue::newFrame(uint64 fence) {
	_currentFrameResourceIndex = (_currentFrameResourceIndex + 1) % _frameResourceItemCount;
	_frameResourceQueue[_currentFrameResourceIndex]->newFrame(fence);
}

std::shared_ptr<FrameResourceItem> FrameResourceQueue::getCurrentFrameResourceItem() const {
	return _frameResourceQueue[_currentFrameResourceIndex];
}

}