#include <dx12lib/Device/Device.h>
#include <dx12lib/Pipeline/RootSignature.h>
#include <optional>

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

RegisterSlot::operator size_t() const {
	return static_cast<size_t>(slot);
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
	assert(!valid());
	ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	ShaderVisibility = visibility;
	Constants.Num32BitValues = static_cast<UINT>(num32BitValues);
	Constants.ShaderRegister = static_cast<UINT>(shaderRegister.slot.getRegisterId());
	Constants.RegisterSpace = static_cast<UINT>(shaderRegister.space);
}

void RootParameter::initAsDescriptorRange(ShaderRegister shaderRegister, UINT count, D3D12_SHADER_VISIBILITY visibility) {
	assert(!valid());
	clear();
	initAsDescriptorTable(1, visibility);
	setTableRange(0, shaderRegister, count);
}

void RootParameter::initAsDescriptorTable(size_t rangeCount, D3D12_SHADER_VISIBILITY visibility) {
	assert(!valid());
	assert(DescriptorTable.pDescriptorRanges == nullptr);
	ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	ShaderVisibility = visibility;
	DescriptorTable.NumDescriptorRanges = static_cast<UINT>(rangeCount);
	DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[rangeCount];
}

void RootParameter::setTableRange(size_t index, ShaderRegister shaderRegister, UINT count) {
	assert(!valid());
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
	_shaderParamLocation.clear();
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

	// �� D3D12_DESCRIPTOR_RANGE ��ƽ��һά������, ͳ�ƺ�ÿ�� Range �����������ͺ�����
	std::memset(&_descriptorTableCount, 0, sizeof(_descriptorTableCount));
	size_t rangeIndex = 0;
	for (size_t rootParamIndex = 0; rootParamIndex < _numRootParams; ++rootParamIndex) {
		const RootParameter &rootParam = _pRootParamArray[rootParamIndex];
		if (rootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
			size_t rangeCount = rootParam.DescriptorTable.NumDescriptorRanges;
			assert(rangeCount <= static_cast<uint8_t>(-1));
			bool isSampler = false;
			if (rangeCount > 0)
				isSampler = rootParam.DescriptorTable.pDescriptorRanges[0].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;

			std::bitset<kMaxDescriptorTables> &bitMask = (isSampler ? _samplerTableBitMask : _descriptorTableBitMask);
			for (size_t j = 0; j < rangeCount; ++j) {
				const D3D12_DESCRIPTOR_RANGE &range = rootParam.DescriptorTable.pDescriptorRanges[j];
				if (range.NumDescriptors <= 0)
					continue;

				// ͳ��ÿ���Ĵ�����ʹ��
				for (size_t offset = 0; offset < range.NumDescriptors; ++offset) {
					size_t baseRegister = range.BaseShaderRegister + offset;
					RegisterSlot slot(range.RangeType, baseRegister);
					RegisterSpace space = static_cast<RegisterSpace>(range.RegisterSpace);
					ShaderRegister shaderRegister(slot, space);
					ShaderParamLocation location(rootParamIndex, rangeIndex, offset);
					_shaderParamLocation[shaderRegister] = location;
				}

				bitMask.set(rangeIndex);
				_descriptorTableCount[rangeIndex] = static_cast<uint8_t>(rangeCount);
				++rangeIndex;
				assert(rangeIndex <= kMaxDescriptorTables);
			}
		}
	} 
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
		return _descriptorTableBitMask;
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		return _samplerTableBitMask;
	default:
		assert(false);
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

std::optional<ShaderParamLocation> RootSignature::getShaderParamLocation(const ShaderRegister &sr) const {
	auto iter = _shaderParamLocation.find(sr);
	if (iter != _shaderParamLocation.end())
		return iter->second;
	return std::nullopt;
}

}
