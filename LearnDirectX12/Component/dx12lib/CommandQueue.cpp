#include "CommandQueue.h"
#include "CommandList.h"
#include "SwapChain.h"
#include "FrameResourceQueue.h"
#include "Device.h"
#include "ResourceStateTracker.h"

namespace dx12lib {

CommandQueue::CommandQueue(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE queueType)
: _queueType(queueType), _pDevice(pDevice), _fenceValue(0) 
{
	auto *pD3DDevice = _pDevice.lock()->getD3DDevice();
	D3D12_COMMAND_QUEUE_DESC desc;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.Type = queueType;
	desc.Priority = 0;
	desc.NodeMask = 0;
	ThrowIfFailed(pD3DDevice->CreateCommandQueue(
		&desc,
		IID_PPV_ARGS(&_pCommandQueue)
	));

	ThrowIfFailed(pD3DDevice->CreateFence(
		0,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&_pFence)
	));

	_pFrameResourceQueue = std::make_unique<FrameResourceQueue>(pDevice, queueType);
}

ID3D12CommandQueue *CommandQueue::getD3D12CommandQueue() const {
	return _pCommandQueue.Get();
}

uint64 CommandQueue::signal(std::shared_ptr<SwapChain> pSwapChain) {
	pSwapChain->present();
	_pCommandQueue->Signal(_pFence.Get(), _fenceValue);
	return _fenceValue;
}

void CommandQueue::executeCommandList(CommandListProxy pCommandList) {
	std::vector<CommandListProxy> cmdLists;
	cmdLists.push_back(pCommandList);
	executeCommandList(cmdLists);
}

void CommandQueue::executeCommandList(const std::vector<CommandListProxy> &cmdLists) {
	std::unordered_set<ID3D12CommandList *> hashset;
	std::vector<ID3D12CommandList *> lists;

	ResourceStateTracker::lock();
	for (auto pCmdList : cmdLists) {
		auto *pD3DCmdList = pCmdList->getD3DCommandList();
		if (hashset.find(pD3DCmdList) != hashset.end())
			continue;
		hashset.insert(pD3DCmdList);
		CommandListProxy pPendingCmdList = createCommandListProxy();
		pCmdList->close(pPendingCmdList._getCommandList());
		pPendingCmdList->close();
		lists.push_back(pPendingCmdList->getD3DCommandList());
		lists.push_back(pD3DCmdList);
	}
	_pCommandQueue->ExecuteCommandLists(static_cast<UINT>(lists.size()), lists.data());
	ResourceStateTracker::unlock();
}

bool CommandQueue::isFenceComplete(uint64 fenceValue) const noexcept {
	return _pFence->GetCompletedValue() >= fenceValue;
}

void CommandQueue::waitForFenceValue(uint64 fenceValue) {
	if (fenceValue != 0 && _pFence->GetCompletedValue() < fenceValue) {
		HANDLE event = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		assert(event != nullptr);
		ThrowIfFailed(_pFence->SetEventOnCompletion(fenceValue, event));
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
}

uint32 CommandQueue::getFrameResourceCount() const {
	return _queueType == D3D12_COMMAND_LIST_TYPE_DIRECT ? 3 : 1;
}

uint64 CommandQueue::getFenceValue() const {
	return _fenceValue;
}

uint64 CommandQueue::getCompletedValue() const {
	return _pFence->GetCompletedValue();
}

CommandListProxy CommandQueue::createCommandListProxy() {
	return _pFrameResourceQueue->createCommandListProxy();
}

void CommandQueue::newFrame() {
	auto pCurrentFrameResourceItem = _pFrameResourceQueue->getCurrentFrameResourceItem();
	waitForFenceValue(pCurrentFrameResourceItem->getFence());
	++_fenceValue;
	_pFrameResourceQueue->newFrame(_fenceValue);
}

void CommandQueue::resize(uint32 width, uint32 height, std::shared_ptr<SwapChain> pSwapChain) {
	waitForFenceValue(_fenceValue);
	newFrame();
	auto pCmdList = createCommandListProxy();
	pSwapChain->resize(pCmdList, width, height);
	executeCommandList(pCmdList);
	_pCommandQueue->Signal(_pFence.Get(), _fenceValue);
	waitForFenceValue(_fenceValue);
}

CommandQueue::~CommandQueue() {
	waitForFenceValue(_fenceValue);
}

}