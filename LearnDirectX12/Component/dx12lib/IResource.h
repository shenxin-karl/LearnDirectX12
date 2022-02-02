#pragma once
#include "dx12libStd.h"
#include <d3d12.h>
#include <wrl.h>

namespace dx12lib {

class IResource {
public:
	virtual WRL::ComPtr<ID3D12Resource> getD3DResource() const = 0;
	virtual ~IResource() = default;
};

}