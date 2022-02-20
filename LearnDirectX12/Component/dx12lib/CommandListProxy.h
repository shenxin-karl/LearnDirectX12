#pragma once
#include <memory>

namespace dx12lib {

class CommandList;
class FrameResourceItem;

class CommandListProxy {
protected:
	CommandListProxy(std::shared_ptr<CommandList> pCmdList);
public:
	CommandList *operator->();
	CommandList &operator*();
	const CommandList *operator->() const;
	const CommandList &operator*() const;
	operator bool() const;
	friend bool operator==(const CommandListProxy &lhs, std::nullptr_t);
	friend bool operator!=(const CommandListProxy &lhs, std::nullptr_t);
private:
	std::shared_ptr<CommandList> _pCmdList;
};

}