#pragma once
#include "dx12libCommon.h"

namespace dx12lib {

class CommandQueue {
public:
	ID3D12CommandQueue *getD3D12CommandQueue() const;
};

}