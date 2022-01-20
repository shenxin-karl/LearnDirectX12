#pragma once
#include <memory>

namespace dx12lib {

class CommandList;
class FrameResourceItem;

class CommandListProxy {
public:
	CommandListProxy(std::shared_ptr<CommandList> pCmdList, std::weak_ptr<FrameResourceItem> pFrameResourceItem);
	CommandListProxy(const CommandListProxy &) = delete;
	CommandListProxy &operator=(CommandListProxy &) = delete;
	CommandListProxy(CommandListProxy &&) = default;
	CommandListProxy &operator=(CommandListProxy &&) = default;
	~CommandListProxy();
	CommandList *operator->() noexcept;
	CommandList &operator*() noexcept;
	const CommandList *operator->() const noexcept;
	const CommandList &operator*() const noexcept;
private:
	std::shared_ptr<CommandList>     _pCmdList;
	std::weak_ptr<FrameResourceItem> _pFrameResourceItem;
};

}