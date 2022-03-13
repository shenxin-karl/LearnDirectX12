#pragma once
#include "dx12libStd.h"
#include "DescriptorAllocation.h"
#include "IResource.h"

namespace dx12lib {

class DefaultBuffer;
class StructedBuffer : IResource {
public:
	D3D12_CPU_DESCRIPTOR_HANDLE getStructedBufferView() const;
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	std::size_t getBufferSize() const;
protected:
	StructedBuffer(std::weak_ptr<Device> pDevice, 
		std::shared_ptr<CommandList> pCmdList, 
		const void *pData, 
		std::size_t sizeInByte
	);
private:
	DescriptorAllocation           _structedBufferView;
	std::unique_ptr<DefaultBuffer> _pDefaultBuffer;
	std::size_t                    _bufferSize;
};

}