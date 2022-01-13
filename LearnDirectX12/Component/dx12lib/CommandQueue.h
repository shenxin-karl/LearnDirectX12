#pragma once
#include "dx12libCommon.h"
#include <mutex>
#include <queue>

namespace dx12lib {

class CommandList;
class CommandQueue {
public:
	ID3D12CommandQueue *getD3D12CommandQueue() const;

private:
	WRL::ComPtr<ID3D12CommandQueue> *_pCommandQueue;

};

}