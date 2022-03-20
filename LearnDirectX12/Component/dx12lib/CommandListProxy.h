#pragma once
#include <memory>
#include "CommandContext.h"
#include "CommandList.h"

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

template<typename T>
class _ContextBase {
public:
	_ContextBase(std::shared_ptr<CommandList> pCmdList) : _pCmdList(pCmdList) {
	}
	T *operator->() {
		return static_cast<T *>(_pCmdList.get());
	}
	T &operator*() {
		return static_cast<T &>(*_pCmdList);
	}
	const T *operator->() const {
		return static_cast<const T *>(_pCmdList.get());
	}
	const T &operator*() const {
		return static_cast<const T &>(*_pCmdList);
	}
	operator bool() const {
		return _pCmdList != nullptr;
	}
	friend bool operator==(const _ContextBase &lhs, std::nullptr_t) {
		return lhs._pCmdList == nullptr;
	}
	friend bool operator!=(const _ContextBase &lhs, std::nullptr_t) {
		return lhs._pCmdList != nullptr;
	}
private:
	std::shared_ptr<CommandList> _pCmdList;
};

class GrahpicsContextProxy : public _ContextBase<GrahpicsContext> {

};

class ComputeContextProxy : public _ContextBase<ComputeContext> {

};

}