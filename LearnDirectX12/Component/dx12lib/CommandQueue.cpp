#include "CommandQueue.h"
#include "Device.h"
#include "CommandList.h"

namespace dx12lib {

CommandQueue::CommandQueue(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE queueType)
: _queueType(queueType), _pDevice(pDevice), _fenceValue(0) 
{
	auto *pD3DDevice = _pDevice.lock()->getD3DDevice();
	D3D12_COMMAND_QUEUE_DESC desc;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.Type = queueType;
	ThrowIfFailed(pD3DDevice->CreateCommandQueue(
		&desc,
		IID_PPV_ARGS(&_pCommandQueue)
	));
}

ID3D12CommandQueue *CommandQueue::getD3D12CommandQueue() const {
	return _pCommandQueue.Get();
}

std::shared_ptr<CommandList> CommandQueue::getCommandList() const {
	if (!_availableCommandLists.empty()) {
		std::shared_ptr<CommandList> pCmdList;
		if (_availableCommandLists.tryPop(pCmdList))
			return pCmdList;
	}
	std::shared_ptr<CommandList> pNewCmdList = std::make_shared<CommandList>(_pDevice, _queueType);
	_commandLists.push(pNewCmdList);
	return pNewCmdList;
}

}