#include <dx12lib/Device/Device.h>
#include <dx12lib/Pipeline/RootSignature.h>

namespace dx12lib {

RootParameter::RootParameter(D3D12_SHADER_VISIBILITY visibility /*= D3D12_SHADER_VISIBILITY_ALL*/)
: _visibility(visibility) {
	_pRanges = std::make_shared<std::vector<D3D12_DESCRIPTOR_RANGE>>();
}

void RootParameter::initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, 
	UINT numDescriptors, 
	UINT baseShaderRegister, 
	UINT registerSpace /*= 0 */) 
{
	CD3DX12_DESCRIPTOR_RANGE range;
	range.Init(rangeType, numDescriptors, baseShaderRegister, registerSpace);
	_pRanges->push_back(range);
	_rootParame.InitAsDescriptorTable(static_cast<UINT>(_pRanges->size()), _pRanges->data(), _visibility);
}

void RootParameter::InitAsConstants(UINT num32BitValues, UINT shaderRegister, UINT registerSpace /*= 0 */) {
	_rootParame.InitAsConstants(num32BitValues, shaderRegister, registerSpace);
}

const CD3DX12_ROOT_PARAMETER &RootParameter::getRootParameDesc() const {
	return _rootParame;
}

RootSignatureDescHelper::RootSignatureDescHelper(D3D12_ROOT_SIGNATURE_FLAGS flag)
: _flag(flag) {
}

RootSignatureDescHelper::RootSignatureDescHelper(const std::vector<D3D12_STATIC_SAMPLER_DESC> &staticSamplers, 
	D3D12_ROOT_SIGNATURE_FLAGS flag /*= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT */)
: _staticSamplers(staticSamplers), _flag(flag) {
}

RootSignatureDescHelper::RootSignatureDescHelper(const std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> &staticSamplers,
	D3D12_ROOT_SIGNATURE_FLAGS flag /*= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT */) 
: _staticSamplers(staticSamplers.begin(), staticSamplers.end()), _flag(flag) {
}

void RootSignatureDescHelper::addRootParameter(const RootParameter &parame) {
	_rootParameterInfos.push_back(parame);
}

D3D12_ROOT_SIGNATURE_DESC RootSignatureDescHelper::getRootSignatureDesc() const {
	_rootParameters.clear();
	for (auto &rootParameter : _rootParameterInfos)
		_rootParameters.push_back(rootParameter.getRootParameDesc());

	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.Init(static_cast<UINT>(_rootParameters.size()),
		_rootParameters.data(),
		static_cast<UINT>(_staticSamplers.size()),
		_staticSamplers.data(),
		_flag
	);
	return rootDesc;
}

void RootSignatureDescHelper::resize(std::size_t num, D3D12_SHADER_VISIBILITY visibility) {
	_rootParameters.clear();
	for (std::size_t i = 0; i < num; ++i)
		_rootParameterInfos.push_back(RootParameter(visibility));
}

dx12lib::RootParameter &RootSignatureDescHelper::operator[](std::size_t index) {
	assert(index < _rootParameterInfos.size());
	return _rootParameterInfos[index];
}

RootSignature::RootSignature(std::weak_ptr<Device> pDevice, const D3D12_ROOT_SIGNATURE_DESC &desc) 
: _pDevice(pDevice) 
{
	setRootSignatureDesc(desc);
}

const D3D12_ROOT_SIGNATURE_DESC &RootSignature::getRootSignatureDesc() const {
	return _rootSignatureDesc;
}

std::bitset<kMaxDescriptorTables> RootSignature::getDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE heapType) {
	switch (heapType) {
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return _descriptorTableBitMask[0];
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		return _descriptorTableBitMask[1];
	default:
		return { 0 };
	}
}

uint32 RootSignature::getNumDescriptorsByType(D3D12_DESCRIPTOR_HEAP_TYPE type, std::size_t rootParameterIndex) const {
	assert(rootParameterIndex < kDynamicDescriptorPerHeap);
	switch (type) {
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return _numDescriptorPerTable[0][rootParameterIndex];
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		return _numDescriptorPerTable[1][rootParameterIndex];
	default:
		return 0;
	}
}

WRL::ComPtr<ID3D12RootSignature> RootSignature::getRootSignature() const {
	return _pRootSignature;
}

void RootSignature::reset() {
	_rootSignatureDesc = CD3DX12_ROOT_SIGNATURE_DESC(D3D12_DEFAULT);
	_pRootSignature = nullptr;
	_descriptorRangeCache.clear();
	for (std::size_t i = 0; i < kDynamicDescriptorHeapCount; ++i) {
		std::fill(_numDescriptorPerTable[i].begin(), _numDescriptorPerTable[i].end(), 0);
		_descriptorTableBitMask[i].reset();
	}
}

void RootSignature::setRootSignatureDesc(const D3D12_ROOT_SIGNATURE_DESC &desc) {
	reset();

	// collect descriptor range
	for (std::size_t i = 0; i < desc.NumParameters; ++i) {
		const auto &rootParameter = desc.pParameters[i];
		_rootParamterCache.push_back(rootParameter);
		if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
			for (std::size_t j = 0; j < rootParameter.DescriptorTable.NumDescriptorRanges; ++j)
				_descriptorRangeCache.push_back(rootParameter.DescriptorTable.pDescriptorRanges[j]);
		}
	}

	// build _rootSignatureDesc
	_rootSignatureDesc = desc;
	_rootSignatureDesc.pParameters = _rootParamterCache.data();
	auto *pBaseDescriptor = _descriptorRangeCache.data();
	for (std::size_t rootIndex = 0; rootIndex < desc.NumParameters; ++rootIndex) {
		const D3D12_ROOT_PARAMETER &rootParameter = _rootSignatureDesc.pParameters[rootIndex];
		if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
			// Force changes to the DescriptorTable
			auto numDescriptor = rootParameter.DescriptorTable.NumDescriptorRanges;
			const_cast<D3D12_ROOT_PARAMETER &>(rootParameter).DescriptorTable = {
				numDescriptor,
				pBaseDescriptor
			};
			pBaseDescriptor += numDescriptor;
		
			std::size_t numRanges = static_cast<std::size_t>(rootParameter.DescriptorTable.NumDescriptorRanges);
			for (std::size_t i = 0; i < numRanges; ++i) {
				const auto &range = rootParameter.DescriptorTable.pDescriptorRanges[i];
				auto index = getPerTableIndexByRangeType(range.RangeType);
				_numDescriptorPerTable[index][rootIndex] += range.NumDescriptors;
				_descriptorTableBitMask[index].set(rootIndex);
			}
		}
	}

	// build root Signature 
	WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
	WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&_rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&serializedRootSig,
		&errorBlob
	);

	if (FAILED(hr)) {
		OutputDebugString(static_cast<const char *>(errorBlob->GetBufferPointer()));
		ThrowIfFailed(hr);
	}

	ThrowIfFailed(_pDevice.lock()->getD3DDevice()->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&_pRootSignature)
	));
}


std::size_t RootSignature::getPerTableIndexByRangeType(D3D12_DESCRIPTOR_RANGE_TYPE type) {
	switch (type) {
	case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
	case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
	case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
		return 0;
	case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
		return 1;
	default:
		assert(false);
		return 0;
	};
}


}
