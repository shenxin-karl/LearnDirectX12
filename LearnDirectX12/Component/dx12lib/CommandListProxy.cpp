#include "CommandListProxy.h"
#include "FrameResourceQueue.h"

namespace dx12lib {

CommandListProxy::CommandListProxy(std::shared_ptr<CommandList> pCmdList, std::weak_ptr<FrameResourceItem> pFrameResourceItem)
: _pCmdList(pCmdList), _pFrameResourceItem(pFrameResourceItem) {
}

CommandListProxy::~CommandListProxy() {
	auto pFrameResourceItem = _pFrameResourceItem.lock();
	if (pFrameResourceItem != nullptr)
		pFrameResourceItem->releaseCommandList(_pCmdList);
}

CommandList *CommandListProxy::operator->() noexcept {
	return _pCmdList.get();
}

CommandList &CommandListProxy::operator*() noexcept {
	return *_pCmdList.get();
}

const CommandList *CommandListProxy::operator->() const noexcept {
	return _pCmdList.get();
}

const CommandList &CommandListProxy::operator*() const noexcept {
	return *_pCmdList.get();
}

}