#pragma once
#include "dx12libStd.h"

namespace dx12lib {

class RootSignature {
public:
	RootSignature(std::weak_ptr<Device> pDevice, const D3D12_ROOT_SIGNATURE_DESC &desc);
	const D3D12_ROOT_SIGNATURE_DESC &getRootSignatureDesc() const;
	std::bitset<kMaxDescriptorTables> getDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
	uint32 getNumDescriptors(std::size_t rootParameterIndex) const;
	void reset();
private:
	void setRootSignatureDesc(const D3D12_ROOT_SIGNATURE_DESC &desc);
private:
	uint32                              _numDescriptorsPerTable[kDynamicDescriptorPerHeap];
	std::weak_ptr<Device>               _pDevice;
	D3D12_ROOT_SIGNATURE_DESC           _rootSignatureDesc;
	WRL::ComPtr<ID3D12RootSignature>    _pRootSignature;
	std::bitset<kMaxDescriptorTables>   _descriptorTableBitMask;
	std::bitset<kMaxDescriptorTables>   _samplerTableBitMask;
	std::vector<D3D12_DESCRIPTOR_RANGE> _descriptorRangeCache;
};

}