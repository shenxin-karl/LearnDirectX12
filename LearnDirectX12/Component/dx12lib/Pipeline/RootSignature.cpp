#include <dx12lib/Device/Device.h>
#include <dx12lib/Pipeline/RootSignature.h>

namespace dx12lib {

RegisterSlot::RegisterSlot(Slot slot) : slot(slot) {
}

RegisterSlot::RegisterSlot(D3D12_DESCRIPTOR_RANGE_TYPE type, size_t baseRegister) {
	assert(baseRegister < 9);
	switch (type) {
	case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
		slot = static_cast<Slot>(CBVBegin + baseRegister);
		break;
	case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
		slot = static_cast<Slot>(UAVBegin + baseRegister);
		break;
	case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
		slot = static_cast<Slot>(SRVBegin + baseRegister);
		break;
	case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
		slot = static_cast<Slot>(SamplerBegin + baseRegister);
		break;
	default:
		assert(false);
		break;
	}
}

bool RegisterSlot::isCBV() const {
	return slot >= CBVBegin && slot < CBVEnd;
}

bool RegisterSlot::isSRV() const {
	return slot >= SRVBegin && slot < SRVEnd;
}

bool RegisterSlot::isUAV() const {
	return slot >= UAVBegin && slot < UAVEnd;
}

bool RegisterSlot::isSampler() const {
	return slot >= SamplerBegin && slot < SamplerEnd;
}

size_t RegisterSlot::getRegisterId() const {
	if (isCBV())
		return slot - CBVBegin;
	if (isSRV())
		return slot - SRVBegin;
	if (isUAV())
		return slot - UAVBegin;
	if (isSampler())
		return slot - SamplerBegin;
	assert(false);
	return static_cast<size_t>(-1);
}

D3D12_DESCRIPTOR_RANGE_TYPE RegisterSlot::getDescriptorRangeType() const {
	if (isCBV())
		return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	if (isSRV())
		return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	if (isUAV())
		return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	if (isSampler())
		return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	assert(false);
	return static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(-1);
}

bool operator==(const RegisterSlot &lhs, const RegisterSlot &rhs) noexcept {
	return lhs.slot == rhs.slot;
}

bool operator!=(const RegisterSlot &lhs, const RegisterSlot &rhs) noexcept {
	return !(lhs == rhs);
}

RegisterSlot operator+(const RegisterSlot &lhs, size_t rhs) noexcept {
	RegisterSlot res(static_cast<RegisterSlot::Slot>(lhs.slot + rhs));
	assert(res.getRegisterId() != static_cast<size_t>(-1));
	return res;
}



////////////////////////////////////////ShaderRegister/////////////////////////////////////////////////

ShaderRegister::ShaderRegister(RegisterSlot slot, RegisterSpace space) : slot(slot), space(space) {
}

bool operator==(const ShaderRegister &lhs, const ShaderRegister &rhs) {
	return lhs.slot == rhs.slot && lhs.space == rhs.space;
}

bool operator!=(const ShaderRegister &lhs, const ShaderRegister &rhs) {
	return !(lhs == rhs);
}

ShaderRegister operator+(ShaderRegister lhs, size_t rhs) noexcept {
	RegisterSlot slot = lhs.slot + rhs;
	return { slot, lhs.space };
}

////////////////////////////////////////RootParameter/////////////////////////////////////////////////

RootParameter::RootParameter() {
	std::memset(this, 0, sizeof(*this));
	ParameterType = kInvalidType;
}

RootParameter::~RootParameter() {
	clear();
}

void RootParameter::clear() {
	if (ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
		assert(DescriptorTable.pDescriptorRanges != nullptr);
		delete[] DescriptorTable.pDescriptorRanges;
		DescriptorTable.pDescriptorRanges = nullptr;
	}
	ParameterType = kInvalidType;
}

bool RootParameter::valid() const {
	return ParameterType != kInvalidType;
}

void RootParameter::initAsConstants(ShaderRegister shaderRegister, size_t num32BitValues, D3D12_SHADER_VISIBILITY visibility) {
	assert(shaderRegister.slot.isCBV());
	ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	ShaderVisibility = visibility;
	Constants.Num32BitValues = static_cast<UINT>(num32BitValues);
	Constants.ShaderRegister = static_cast<UINT>(shaderRegister.slot.getRegisterId());
	Constants.RegisterSpace = static_cast<UINT>(shaderRegister.space);
}

void RootParameter::initAsDescriptorRange(ShaderRegister shaderRegister, UINT count, D3D12_SHADER_VISIBILITY visibility) {
	clear();
	initAsDescriptorTable(1, visibility);
	setTableRange(0, shaderRegister, count);
}

void RootParameter::initAsDescriptorTable(size_t rangeCount, D3D12_SHADER_VISIBILITY visibility) {
	assert(DescriptorTable.pDescriptorRanges == nullptr);
	ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	ShaderVisibility = visibility;
	DescriptorTable.NumDescriptorRanges = static_cast<UINT>(rangeCount);
	DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[rangeCount];
}

void RootParameter::setTableRange(size_t index, ShaderRegister shaderRegister, UINT count) {
	assert(DescriptorTable.pDescriptorRanges != nullptr);
	assert(index < DescriptorTable.NumDescriptorRanges);
	D3D12_DESCRIPTOR_RANGE &range = const_cast<D3D12_DESCRIPTOR_RANGE &>(DescriptorTable.pDescriptorRanges[index]);
	range.RangeType = shaderRegister.slot.getDescriptorRangeType();
	range.NumDescriptors = count;
	range.BaseShaderRegister = shaderRegister.slot.getRegisterId();
	range.RegisterSpace = static_cast<UINT>(shaderRegister.space);
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

////////////////////////////////////////RootSignature/////////////////////////////////////////////////

RootSignature::RootSignature(std::weak_ptr<Device> pDevice) : _pDevice(pDevice) {
}

RootSignature::RootSignature(std::weak_ptr<Device> pDevice, size_t numRootParams, size_t numStaticSamplers) : _pDevice(pDevice) {
	reset(numRootParams, numStaticSamplers);
}

void RootSignature::reset(size_t numRootParams, size_t numStaticSamplers) {
	assert(numRootParams > 0);
	_finalized = false;
	_numRootParams = numRootParams;
	_numStaticSamplers = numStaticSamplers;
	_pRootParamArray = std::make_unique<RootParameter[]>(numRootParams);
	if (numStaticSamplers > 0)
		_pStaticSamplerArray = std::make_unique<D3D12_STATIC_SAMPLER_DESC[]>(numStaticSamplers);
}

void RootSignature::finalize(D3D12_ROOT_SIGNATURE_FLAGS flags) {
	if (_finalized) {
		assert(false);
		return;
	}

	D3D12_ROOT_SIGNATURE_DESC rootDesc(
		_numRootParams, 
		_pRootParamArray.get(), 
		_numStaticSamplers, 
		_pStaticSamplerArray.get(),
		flags
	);

	// build root Signature 
	WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
	WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootDesc,
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
	_finalized = true;
}

void RootSignature::initStaticSampler(size_t index, const D3D12_STATIC_SAMPLER_DESC &desc) {
	assert(index < _numStaticSamplers);
	_pStaticSamplerArray[index] = desc;
}

WRL::ComPtr<ID3D12RootSignature> RootSignature::getRootSignature() const {
	return _pRootSignature;
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

RootParameter &RootSignature::operator[](size_t index) {
	assert(index < _numRootParams);
	return _pRootParamArray[index];
}

const RootParameter &RootSignature::operator[](size_t index) const {
	assert(index < _numRootParams);
	return _pRootParamArray[index];
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
