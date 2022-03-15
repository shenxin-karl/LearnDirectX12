#pragma once
#include "dx12libStd.h"
#include "IResource.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class ShaderResourceBuffer : public IResource {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	~ShaderResourceBuffer() override;
protected:
	void trackResource() override;
protected:
	
};

}