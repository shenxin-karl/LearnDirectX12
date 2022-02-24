#pragma once
#include "dx12libStd.h"

namespace dx12lib {


class RootParameter {
public:
	RootParameter(D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
	RootParameter(const RootParameter &) = default;
	void initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
		UINT numDescriptors,
		UINT baseShaderRegister,
		UINT registerSpace = 0
	);

	const CD3DX12_ROOT_PARAMETER &getRootParameDesc() const;
private:
	CD3DX12_ROOT_PARAMETER _rootParame;
	D3D12_SHADER_VISIBILITY _visibility;
	std::shared_ptr<std::vector<D3D12_DESCRIPTOR_RANGE>> _pRanges;
};

class RootSignatureDescHelper {
public:
	RootSignatureDescHelper(D3D12_ROOT_SIGNATURE_FLAGS flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	RootSignatureDescHelper(const std::vector<D3D12_STATIC_SAMPLER_DESC> &staticSamplers,
		D3D12_ROOT_SIGNATURE_FLAGS flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);
	RootSignatureDescHelper(const std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> &staticSamplers,
		D3D12_ROOT_SIGNATURE_FLAGS flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);
	RootSignatureDescHelper(const RootSignatureDescHelper &) = delete;
	RootSignatureDescHelper &operator=(const RootSignatureDescHelper &) = delete;
	void addRootParameter(const RootParameter &parame);
	D3D12_ROOT_SIGNATURE_DESC getRootSignatureDesc() const;
	void resize(std::size_t num, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
	RootParameter &operator[](std::size_t index);
private:
	std::vector<RootParameter>					_rootParameterInfos;
	mutable std::vector<D3D12_ROOT_PARAMETER>   _rootParameters;
	std::vector<D3D12_STATIC_SAMPLER_DESC>		_staticSamplers;
	D3D12_ROOT_SIGNATURE_FLAGS					_flag;
};

class RootSignature {
protected:
	RootSignature(std::weak_ptr<Device> pDevice, const D3D12_ROOT_SIGNATURE_DESC &desc);
public:
	const D3D12_ROOT_SIGNATURE_DESC &getRootSignatureDesc() const;
	std::bitset<kMaxDescriptorTables> getDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
	uint32 getNumDescriptorsByType(D3D12_DESCRIPTOR_HEAP_TYPE type, std::size_t rootParameterIndex) const;
	WRL::ComPtr<ID3D12RootSignature> getRootSignature() const;
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