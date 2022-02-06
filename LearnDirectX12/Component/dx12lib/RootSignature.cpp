#include "RootSignature.h"

namespace dx12lib {

const D3D12_ROOT_SIGNATURE_DESC &RootSignature::getRootSignatureDesc() const {
	return _rootSignatureDesc;
}

std::bitset<kMaxDescriptorTables> RootSignature::getDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE heapType) {
	switch (heapType)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return _descriptorTableBitMask;
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		return _samplerTableBitMask;
	default:
		return { 0 };
	}
}

uint32 RootSignature::getNumDescriptors(std::size_t rootParameterIndex) const {
	assert(rootParameterIndex < kMaxDescriptorTables);
	return _numDescriptorsPerTable[rootParameterIndex];
}

void RootSignature::reset() {
	std::fill(std::begin(_numDescriptorsPerTable), std::end(_numDescriptorsPerTable), 0);
	_rootSignatureDesc = CD3DX12_ROOT_SIGNATURE_DESC(D3D12_DEFAULT);
	_pRootSignature = nullptr;
	_descriptorTableBitMask.reset();
	_samplerTableBitMask.reset();
	_descriptorRangeCache.clear();
}

void RootSignature::setRootSignatureDesc(const D3D12_ROOT_SIGNATURE_DESC &desc) {
	reset();

	for (std::size_t i = 0; i < desc.NumParameters; ++i) {
		const auto &rootParameter = desc.pParameters[i];
		if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
			for (std::size_t j = 0; j < rootParameter.DescriptorTable.NumDescriptorRanges; ++j)
				_descriptorRangeCache.push_back(rootParameter.DescriptorTable.pDescriptorRanges[j]);
		}
	}

	_rootSignatureDesc = desc;
	auto *pBaseDescriptor = _descriptorRangeCache.data();
	for (std::size_t i = 0; i < desc.NumParameters; ++i) {
		const D3D12_ROOT_PARAMETER &rootParameter = _rootSignatureDesc.pParameters[i];
		if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
			/// Force changes to the DescriptorTable
			auto numDescriptor = rootParameter.DescriptorTable.NumDescriptorRanges;
			const_cast<D3D12_ROOT_PARAMETER &>(rootParameter).DescriptorTable = {
				numDescriptor,
				pBaseDescriptor
			};
			pBaseDescriptor += numDescriptor;
			_numDescriptorsPerTable[i] = numDescriptor;

			if (rootParameter.DescriptorTable.NumDescriptorRanges > 0) {
				auto rangeType = rootParameter.DescriptorTable.pDescriptorRanges[0].RangeType;
				switch (rangeType)
				{
				case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
				case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
				case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
					_descriptorTableBitMask.set(i);
					break;
				case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
					_samplerTableBitMask.set(i);
					break;
				}
			}
	
		}
	}
}

}
