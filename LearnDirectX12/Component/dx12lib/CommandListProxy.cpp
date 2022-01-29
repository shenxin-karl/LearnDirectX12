#include "CommandListProxy.h"
#include "FrameResourceQueue.h"

namespace dx12lib {

CommandListProxy::CommandListProxy(std::shared_ptr<CommandList> pCmdList, std::weak_ptr<FrameResourceItem> pFrameResourceItem) {
	_pSharedBuffer = std::make_shared<SharedBuffer>(pCmdList, pFrameResourceItem);
}

CommandList *CommandListProxy::operator->() {
	return _pSharedBuffer->_pCmdList.get();
}

const CommandList *CommandListProxy::operator->() const {
	return _pSharedBuffer->_pCmdList.get();
}

CommandList &CommandListProxy::operator*() {
	return *_pSharedBuffer->_pCmdList.get();
}

const CommandList &CommandListProxy::operator*() const {
	return *_pSharedBuffer->_pCmdList.get();
}

CommandListProxy::SharedBuffer::~SharedBuffer() {
	auto pSharedFrameResourceItem = _pFreamResourceItem.lock();
	if (pSharedFrameResourceItem != nullptr)
		pSharedFrameResourceItem->releaseCommandList(_pCmdList);
}

CommandListProxy::operator bool() const {
	return _pSharedBuffer->_pCmdList.operator bool();
}

bool operator==(const CommandListProxy &lhs, std::nullptr_t) {
	return lhs._pSharedBuffer->_pCmdList == nullptr;
}

bool operator!=(const CommandListProxy &lhs, std::nullptr_t) {
	return lhs._pSharedBuffer->_pCmdList != nullptr;
}


}