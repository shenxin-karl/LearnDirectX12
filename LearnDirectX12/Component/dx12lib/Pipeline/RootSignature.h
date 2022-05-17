#pragma once
#include <dx12lib/dx12libStd.h>

namespace dx12lib {

struct RegisterSlot {
	enum Slot : size_t {
		NONE = -1,
		CBVBegin = 0,	SRVBegin = 10,	UAVBegin = 20,	SamplerBegin = 30,		
		CBV0 = 0,		SRV0 = 10,		UAV0 = 20,		Sampler0 = 30,
		CBV1 = 1,		SRV1 = 11,		UAV1 = 21,		Sampler1 = 31,
		CBV2 = 2,		SRV2 = 12,		UAV2 = 22,		Sampler2 = 32,
		CBV3 = 3,		SRV3 = 13,		UAV3 = 23,		Sampler3 = 33,
		CBV4 = 4,		SRV4 = 14,		UAV4 = 24,		Sampler4 = 34,
		CBV5 = 5,		SRV5 = 15,		UAV5 = 25,		Sampler5 = 35,
		CBV6 = 6,		SRV6 = 16,		UAV6 = 26,		Sampler6 = 36,
		CBV7 = 7,		SRV7 = 17,		UAV7 = 27,		Sampler7 = 37,
		CBV8 = 8,		SRV8 = 18,		UAV8 = 28,		Sampler8 = 38,
		CBVEnd = 9,		SRVEnd = 19,	UAVEnd = 29,	SamplerEnd = 39,
	};
public:
	RegisterSlot(Slot slot);
	RegisterSlot(D3D12_DESCRIPTOR_RANGE_TYPE type, size_t baseRegister);
	RegisterSlot(const RegisterSlot &) = default;
	bool isCBV() const;
	bool isSRV() const;
	bool isUAV() const;
	bool isSampler() const;
	size_t getRegisterId() const;
	D3D12_DESCRIPTOR_RANGE_TYPE getDescriptorRangeType() const;
	friend bool operator==(const RegisterSlot &lhs, const RegisterSlot &rhs) noexcept;
	friend bool operator!=(const RegisterSlot &lhs, const RegisterSlot &rhs) noexcept;
	friend RegisterSlot operator+(const RegisterSlot &lhs, size_t rhs) noexcept;
	explicit operator size_t() const;
public:
	Slot slot;
};

struct ShaderRegister {
	RegisterSlot  slot;
	RegisterSpace space;
public:
	ShaderRegister(const ShaderRegister &) = default;
	ShaderRegister(ShaderRegister &&) noexcept = default;
	ShaderRegister &operator=(const ShaderRegister &) = default;
	ShaderRegister &operator=(ShaderRegister &&) noexcept = default;
	ShaderRegister(RegisterSlot slot, RegisterSpace space = RegisterSpace::Space0);
	friend bool operator==(const ShaderRegister &lhs, const ShaderRegister &rhs);
	friend bool operator!=(const ShaderRegister &lhs, const ShaderRegister &rhs);
	friend ShaderRegister operator+(ShaderRegister lhs, size_t rhs) noexcept;
};

struct ShaderRegisterHasher {
	size_t operator()(const ShaderRegister &shaderRegister) const noexcept {
		std::hash<size_t> hasher;
		size_t hashValue0 = hasher(static_cast<size_t>(shaderRegister.slot));
		size_t hashValue1 = hasher(static_cast<size_t>(shaderRegister.space));
		return hashValue0 ^ (hashValue1 >> 1);
	}
};

class RootParameter : protected D3D12_ROOT_PARAMETER {
	constexpr static D3D12_ROOT_PARAMETER_TYPE kInvalidType = static_cast<D3D12_ROOT_PARAMETER_TYPE>(-1);
	friend class RootSignature;
public:
	RootParameter();
	RootParameter(const RootParameter &) = delete;
	RootParameter &operator=(const RootParameter &) = delete;
	~RootParameter();
	void clear();
	bool valid() const;
	void initAsConstants(ShaderRegister shaderRegister, size_t num32BitValues, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
	void initAsDescriptorRange(ShaderRegister shaderRegister, UINT count, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
	void initAsDescriptorTable(size_t rangeCount, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
	void setTableRange(size_t index, ShaderRegister shaderRegister, UINT count);
};

struct ShaderParamLocation {
	size_t rootParamIndex;			// 根参数
	size_t rangeIndex;				// 第几个跟描述符表
	size_t offset;					// 在根描述符表中的位置
};

class RootSignature {
protected:
	explicit RootSignature(std::weak_ptr<Device> pDevice);
	RootSignature(std::weak_ptr<Device> pDevice, size_t numRootParams, size_t numStaticSamplers = 0);
public:
	void reset(size_t numRootParams, size_t numStaticSamplers = 0);
	void finalize(D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);
	void initStaticSampler(size_t index, const D3D12_STATIC_SAMPLER_DESC &desc);
	WRL::ComPtr<ID3D12RootSignature> getRootSignature() const;
	std::bitset<kMaxDescriptorTables> getDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
	RootParameter &operator[](size_t index);
	const RootParameter &operator[](size_t index) const;
	std::optional<ShaderParamLocation> getShaderParamLocation(const ShaderRegister &sr) const;
private:
	using DescriptorsPerTable = std::array<uint8_t, kMaxDescriptorTables>;
	using ShaderParamLocationMap = std::unordered_map<ShaderRegister, ShaderParamLocation, ShaderRegisterHasher>;
private:
	bool _finalized = false;
	size_t _numRootParams;
	size_t _numStaticSamplers;
	std::weak_ptr<Device> _pDevice;
	DescriptorsPerTable _descriptorTableCount;
	ShaderParamLocationMap _shaderParamLocation;
	std::bitset<kMaxDescriptorTables> _descriptorTableBitMask;
	std::bitset<kMaxDescriptorTables> _samplerTableBitMask;
	WRL::ComPtr<ID3D12RootSignature> _pRootSignature;
	std::unique_ptr<RootParameter[]> _pRootParamArray;
	std::unique_ptr<D3D12_STATIC_SAMPLER_DESC[]> _pStaticSamplerArray;
};

}

namespace std {

template<>
struct hash<dx12lib::ShaderRegister> {
	using result_type = std::size_t;
	result_type operator()(const dx12lib::ShaderRegister &s) const noexcept {
		dx12lib::ShaderRegisterHasher hasher;
		return hasher(s);
	}
};

}