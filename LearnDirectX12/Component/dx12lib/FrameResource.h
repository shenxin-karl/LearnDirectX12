#pragma once
#include "dx12libCommon.h"

namespace dx12lib {

class FrameResource {
public:
	FrameResource(const FrameResource &) = delete;
	FrameResource &operator=(const FrameResource &) = delete;
	FrameResource(D3D12_COMMAND_LIST_TYPE cmdListType);
	uint64 getFence() const noexcept;
	void setFence(uint64 fence) const noexcept;
	WRL::ComPtr<ID3D12CommandAllocator> getCmdListAllocator() const noexcept;
private:
	uint64 _fence = 0;
	WRL::ComPtr<ID3D12CommandAllocator> _pCmdListAlloc;
	D3D12_COMMAND_LIST_TYPE _cmdListAllocType;
};

}