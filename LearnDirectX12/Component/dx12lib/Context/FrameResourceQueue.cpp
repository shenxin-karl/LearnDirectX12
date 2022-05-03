#include "CommandList.h"
#include "FrameResourceQueue.h"
#include "Device.h"
#include "MakeObejctTool.hpp"

namespace dx12lib {

FrameResourceItem::FrameResourceItem(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE cmdListType, uint32 frameIndex)
: _fence(0), _frameIndex(frameIndex), _pDevice(pDevice), _cmdListType(cmdListType) {
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

uint32 FrameResourceItem::getFrameIndex() const {
	return _frameIndex;
}

FrameResourceQueue::FrameResourceQueue(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE cmdListType) {
	_frameResourceItemCount = cmdListType == D3D12_COMMAND_LIST_TYPE_DIRECT ? kFrameResourceCount : 1;
	for (uint32 i = 0; i < _frameResourceItemCount; ++i)
		_frameResourceQueue[i] = std::make_shared<dx12libTool::MakeFrameResourceItem>(pDevice, cmdListType, i);
}

std::shared_ptr<CommandList> FrameResourceQueue::createCommandList() {
	return _frameResourceQueue[FrameIndexProxy::getConstantFrameIndexRef()]->createCommandList();
}

uint32 FrameResourceQueue::getMaxFrameResourceCount() const noexcept {
	return _frameResourceItemCount;
}


void FrameResourceQueue::newFrame(uint64 fence) {
	FrameIndexProxy::startNewFrame();
	_frameResourceQueue[FrameIndexProxy::getConstantFrameIndexRef()]->newFrame(fence);
}

std::shared_ptr<FrameResourceItem> FrameResourceQueue::getCurrentFrameResourceItem() const {
	return _frameResourceQueue[FrameIndexProxy::getConstantFrameIndexRef()];
}

}