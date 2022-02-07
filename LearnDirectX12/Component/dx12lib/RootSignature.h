#pragma once
#include "dx12libStd.h"

namespace dx12lib {

class RootSignature {
public:
	RootSignature(std::weak_ptr<Device> pDevice, const D3D12_ROOT_SIGNATURE_DESC &desc);
	const D3D12_ROOT_SIGNATURE_DESC &getRootSignatureDesc() const;
	std::bitset<kMaxDescriptorTables> getDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
	uint32 getNumDescriptorsByType(D3D12_DESCRIPTOR_HEAP_TYPE type, std::size_t rootParameterIndex) const;
	void reset();
private:
	void setRootSignatureDesc(const D3D12_ROOT_SIGNATURE_DESC &desc);
	static std::size_t getPerTableIndexByRangeType(D3D12_DESCRIPTOR_RANGE_TYPE type);
	using DescriptorsPerTable = std::array<uint32, kDynamicDescriptorPerHeap>;
private:
	std::weak_ptr<Device>               _pDevice;
	D3D12_ROOT_SIGNATURE_DESC           _rootSignatureDesc;
	WRL::ComPtr<ID3D12RootSignature>    _pRootSignature;
	std::vector<D3D12_DESCRIPTOR_RANGE> _descriptorRangeCache;

	// 0: CBV_SRV_UAV
	// 1: Sampler
	DescriptorsPerTable                 _numDescriptorPerTable[kDynamicDescriptorHeapCount];
	std::bitset<kMaxDescriptorTables>   _descriptorTableBitMask[kDynamicDescriptorHeapCount];
};

}