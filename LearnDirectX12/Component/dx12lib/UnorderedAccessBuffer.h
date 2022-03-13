#pragma once
#include "dx12libStd.h"
#include "IResource.h"

namespace dx12lib {
	
class UnorderedAccessBuffer : public IResource {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	std::size_t getBufferSize() const;
	~UnorderedAccessBuffer();
protected:
	UnorderedAccessBuffer(std::weak_ptr<Device> pDevice, std::size_t sizeInByte);
private:
	std::size_t _bufferSize;
	WRL::ComPtr<ID3D12Resource> _pResource;
};

}