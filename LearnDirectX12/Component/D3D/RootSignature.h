#pragma once
#include "d3dutil.h"
#include <bitset>
#include <variant>
#include <memory>

namespace d3dUtil {

struct RootDescriptor {
	D3D12_DESCRIPTOR_HEAP_TYPE  _heapType;
};

struct RootDescriptorTable {
	struct Item {
		uint32  _numDescriptors;
		D3D12_DESCRIPTOR_HEAP_TYPE  _heapType;
	};
public:
	std::vector<Item>  _items;
};

struct RootConstantValue {
	uint32 num32BitValue;
};

struct RootParameter {
	uint32  shaderRegister;
	uint32  registerSpace;
	std::variant<RootDescriptor, RootDescriptorTable, RootConstantValue>  parameter;
};

class RootSignature {
public:
	RootSignature(ID3D12Device *pDevice);
	WRL::ComPtr<ID3D12RootSignature> getRootSignature() const;
	CD3DX12_ROOT_SIGNATURE_DESC getRootSignatureDesc() const;
	std::bitset<32> getDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE heapType) const;
	uint32 getNumDescriptors(uint32 rootIndex) const;

	void buildRootSignature(const std::vector<D3D12_ROOT_PARAMETER> &rootParam,
		const std::vector<D3D12_STATIC_SAMPLER_DESC> &staticSampler);
	void buildRootSignature(const std::vector<D3D12_ROOT_PARAMETER> &rootParam,
		const std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> &defaultStaticSampler = getStaticSamplers());

	void reset();
	bool isValid() const;
private:
	ID3D12Device  *_pDevice;
	WRL::ComPtr<ID3D12RootSignature>  _pRootSignature;
	std::vector<D3D12_ROOT_PARAMETER>  _rootParamenter;
	std::vector<D3D12_STATIC_SAMPLER_DESC> _rootStaticSampler;
	std::array<uint32, 32>  _numDescriptorsPreTable;
	std::bitset<32>  _descriptorTableBitMask;
	std::bitset<32>  _samplerTableBitMask;
};

}