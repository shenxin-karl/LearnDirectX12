#include "dx12libStd.h"

namespace dx12lib {

CommandQueueType toCommandQueueType(D3D12_COMMAND_LIST_TYPE type) {
	switch (type) {
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		return CommandQueueType::Direct;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		return CommandQueueType::Compute;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		return CommandQueueType::Copy;
	case D3D12_COMMAND_LIST_TYPE_BUNDLE:
	case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
	case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
	case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
	default:
		return CommandQueueType::None;
	}
}

}