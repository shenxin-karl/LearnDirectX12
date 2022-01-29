#include "CommandQueue.h"
#include "CommandList.h"
#include "SwapChain.h"
#include "FrameResourceQueue.h"
#include "Device.h"

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
	ThrowIfFailed(pSwapChain->present());
	auto fence = ++_fenceValue;
	_pCommandQueue->Signal(_pFence.Get(), fence);
	return fence;
}

void CommandQueue::executeCommandList(CommandListProxy pCommandList) {
	ThrowIfFailed(pCommandList->close());
	ID3D12CommandList *cmdList[] = { pCommandList->getD3DCommandList() };
	_pCommandQueue->ExecuteCommandLists(1, cmdList);
}

void CommandQueue::executeCommandList(const std::vector<CommandListProxy> &cmdLists) {
	std::unordered_set<ID3D12CommandList *> hashset;
	std::vector<ID3D12CommandList *> lists;
	for (auto pCmdList : cmdLists) {
		auto *pD3DCmdList = pCmdList->getD3DCommandList();
		if (hashset.find(pD3DCmdList) != hashset.end())
			continue;
		hashset.insert(pD3DCmdList);
		if (auto *pD3DGraphicsCmdList = dynamic_cast<ID3D12GraphicsCommandList *>(pD3DCmdList))
			ThrowIfFailed(pD3DGraphicsCmdList->Close());
		lists.push_back(pD3DCmdList);
	}
	_pCommandQueue->ExecuteCommandLists(static_cast<UINT>(lists.size()), lists.data());
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

CommandListProxy CommandQueue::createCommandListProxy() {
	return _pFrameResourceQueue->createCommandListProxy();
}

void CommandQueue::newFrame() {
	waitForFenceValue(_fenceValue);
	_pFrameResourceQueue->newFrame(_fenceValue);
}

CommandQueue::~CommandQueue() {
	waitForFenceValue(_fenceValue);
}

}