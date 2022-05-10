#pragma once
#include <dx12lib/Device/Device.h>
#include <dx12lib/Buffer/UploadBuffer.h>
#include <dx12lib/Resource/ResourceView.hpp>
#include <dx12lib/Resource/IResource.h>

namespace dx12lib {

class AppendStructuredBuffer : public IAppendStructuredBuffer {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	size_t getBufferSize() const override;
	size_t getElementCount() const override;
	size_t getElementStride() const override;
	UnorderedAccessView getUAV() const override;
	ShaderResourceView getSRV() const override;
protected:
	AppendStructuredBuffer(std::weak_ptr<Device> pDevice, size_t numElements, size_t stride);
private:
	size_t _elementStride;
	ShaderResourceView _srv;
	UnorderedAccessView _uav;
	std::unique_ptr<DefaultBuffer> _pDefaultBuffer;
};

}
