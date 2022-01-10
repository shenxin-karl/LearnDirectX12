#include "RootSignature.h"

namespace d3dUtil {

RootSignature::RootSignature(ID3D12Device *pDevice)
: _pDevice(pDevice) {
	reset();
}

WRL::ComPtr<ID3D12RootSignature> RootSignature::getRootSignature() const {
	return _pRootSignature;
}

CD3DX12_ROOT_SIGNATURE_DESC RootSignature::getRootSignatureDesc() const {
	return CD3DX12_ROOT_SIGNATURE_DESC(
		static_cast<UINT>(_rootParamenter.size()),
		_rootParamenter.data(),
		static_cast<UINT>(_rootStaticSampler.size()),
		_rootStaticSampler.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);
}

std::bitset<32> RootSignature::getDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE heapType) const {
	switch (heapType) {
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return _descriptorTableBitMask;
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		return _samplerTableBitMask;
	default:
		throw std::exception("RootSignature::getDescriptorTableBitMask: error heap type");
	}
}

uint32 RootSignature::getNumDescriptors(uint32 rootIndex) const {
	assert(rootIndex < _numDescriptorsPreTable.size());
	return _numDescriptorsPreTable[rootIndex];
}

void RootSignature::buildRootSignature(const std::vector<D3D12_ROOT_PARAMETER>& rootParam, 
	const std::vector<D3D12_STATIC_SAMPLER_DESC> &staticSampler) 
{
	reset();

	// initlaize root parameter
	for (std::size_t i = 0; i < rootParam.size(); ++i) {
		_rootParamenter.push_back(rootParam[i]);
		if (rootParam[i].ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			throw std::exception("RootSignature::buildRootSignature::rootParam is not descriptor table");

		UINT numDescriptorRanges = rootParam[i].DescriptorTable.NumDescriptorRanges;

	}
}

void RootSignature::reset() {
	_pRootSignature.Reset();
	_rootParamenter.clear();
	_rootStaticSampler.clear();
	_descriptorTableBitMask.reset();;
	_samplerTableBitMask.reset();

	for (auto &v : _numDescriptorsPreTable)
		v = 0;
}

bool RootSignature::isValid() const {
	return _pRootSignature != nullptr;
}

}