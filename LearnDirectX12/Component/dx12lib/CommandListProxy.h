#pragma once
#include <memory>

namespace dx12lib {

class CommandList;
class FrameResourceItem;

class CommandListProxy {
public:
	CommandListProxy(std::shared_ptr<CommandList> pCmdList, std::weak_ptr<FrameResourceItem> pFrameResourceItem);
	CommandList *operator->();
	CommandList &operator*();
	const CommandList *operator->() const;
	const CommandList &operator*() const;
	operator bool() const;
	friend bool operator==(const CommandListProxy &lhs, std::nullptr_t);
	friend bool operator!=(const CommandListProxy &lhs, std::nullptr_t);
	std::shared_ptr<CommandList> _getCommandList() const;
private:
	struct SharedBuffer {
		std::shared_ptr<CommandList>     _pCmdList;
		std::weak_ptr<FrameResourceItem> _pFreamResourceItem;
		~SharedBuffer();
	};
	std::shared_ptr<SharedBuffer> _pSharedBuffer;
};

}