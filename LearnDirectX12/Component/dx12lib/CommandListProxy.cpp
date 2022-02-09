#include "CommandListProxy.h"
#include "FrameResourceQueue.h"

namespace dx12lib {

CommandListProxy::CommandListProxy(std::shared_ptr<CommandList> pCmdList) {
	_pCmdList = pCmdList;
}

CommandList *CommandListProxy::operator->() {
	return _pCmdList.get();
}

const CommandList *CommandListProxy::operator->() const {
	return _pCmdList.get();
}

CommandList &CommandListProxy::operator*() {
	return *_pCmdList.get();
}

const CommandList &CommandListProxy::operator*() const {
	return *_pCmdList.get();
}

CommandListProxy::operator bool() const {
	return _pCmdList != nullptr;
}

bool operator==(const CommandListProxy &lhs, std::nullptr_t) {
	return lhs._pCmdList == nullptr;
}

bool operator!=(const CommandListProxy &lhs, std::nullptr_t) {
	return lhs._pCmdList != nullptr;
}


}