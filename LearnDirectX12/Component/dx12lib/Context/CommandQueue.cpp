#include <dx12lib/Context/CommandQueue.h>
#include <dx12lib/Context/CommandList.h>
#include <dx12lib/Context/FrameResourceQueue.h>
#include <dx12lib/Context/ContextProxy.hpp>
#include <dx12lib/Device/SwapChain.h>
#include <dx12lib/Device/Device.h>
#include <dx12lib/Resource/ResourceStateTracker.h>
#include <dx12lib/Tool/MakeObejctTool.hpp>

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

	_pFrameResourceQueue = std::make_unique<dx12libTool::MakeFrameResourceQueue>(pDevice, queueType);
}

ID3D12CommandQueue *CommandQueue::getD3D12CommandQueue() const {
	return _pCommandQueue.Get();
}

uint64 CommandQueue::signal(std::shared_ptr<SwapChain> pSwapChain) {
	ThrowIfFailed(_pCommandQueue->Signal(_pFence.Get(), _fenceValue));
	pSwapChain->present();
	return _fenceValue;
}

void CommandQueue::executeCommandList(ContextProxy pContext) {
	std::vector<ContextProxy> cmdLists;
	cmdLists.push_back(pContext);
	executeCommandList(cmdLists);
}

void CommandQueue::executeCommandList(const std::vector<ContextProxy> &contextList) {
	std::unordered_set<ID3D12CommandList *> hashset;
	std::vector<ID3D12CommandList *> lists;

	ResourceStateTracker::lock();
	for (auto &pContext : contextList) {
		auto pCmdList = pContext.getCmdList();
		auto *pD3DCmdList = pCmdList->getD3DCommandList();
		if (hashset.find(pD3DCmdList) != hashset.end())
			continue;
		hashset.insert(pD3DCmdList);
		std::shared_ptr<CommandList> pPendingCmdList = _pFrameResourceQueue->createCommandList();
		pCmdList->close(pPendingCmdList);
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
	auto completedValue = _pFence->GetCompletedValue();
	if (fenceValue != 0 && completedValue < fenceValue) {
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

DirectContextProxy CommandQueue::createDirectContextProxy() {
	return _pFrameResourceQueue->createCommandList();
}

void CommandQueue::newFrame() {
	auto pCurrentFrameResourceItem = _pFrameResourceQueue->getCurrentFrameResourceItem();
	waitForFenceValue(pCurrentFrameResourceItem->getFence());
	++_fenceValue;
	_pFrameResourceQueue->newFrame(_fenceValue);
}

void CommandQueue::flushCommandQueue() {
	++_fenceValue;
	ThrowIfFailed(_pCommandQueue->Signal(_pFence.Get(), _fenceValue));
	waitForFenceValue(_fenceValue);
}

CommandQueue::~CommandQueue() {
	waitForFenceValue(_fenceValue);
}

FrameResourceQueue *CommandQueue::getFrameResourceQueue() {
	return _pFrameResourceQueue.get();
}

}