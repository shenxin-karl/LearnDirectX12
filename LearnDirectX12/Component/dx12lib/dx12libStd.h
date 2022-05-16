#pragma once
#define NOMINMAX
#include <cassert>
#include <cstdint>
#include <wrl.h>
#include <exception>
#include "Tool/D3Dx12.h"
#include <comdef.h>
#include <Windows.h>
#include <d3dcompiler.h>
#include <memory>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <deque>
#include <queue>
#include <source_location>
#include <mutex>
#include <dxgi.h>
#include <DirectXMath.h>
#include <bitset>
#include <functional>
#include <array>
#include "ComponentStd.h"

#ifndef interface
	#define interface struct
#endif

namespace DX = DirectX;
namespace WRL = Microsoft::WRL;

namespace dx12lib {

namespace WRL = Microsoft::WRL;
namespace DX = DirectX;

using uint8 = std::uint8_t;
using uin16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using int8 = std::int8_t;
using in16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using size_t = std::size_t;

enum class CubeFace : size_t {
	Right		= 0,
	Left		= 1,
	Top			= 2,
	Bottom		= 3,
	Back		= 4,
	Front		= 5,
	POSITIVE_X	= 0,
	NEGATIVE_X	= 1,
	POSITIVE_Y	= 2,
	NEGATIVE_Y	= 3,
	POSITIVE_Z	= 4,
	NEGATIVE_Z	= 5,
};

enum class ShaderResourceDimension {
	Texture2D,
	Texture2DArray,
	Texture3D,
	TextureCube,
};

enum class BufferType {
	DefaultBuffer,
	UploadBuffer,
	ConstantBuffer,
	VertexBuffer,
	IndexBuffer,
	StructuredBuffer,
	ConsumeStructuredBuffer,
	AppendStructuredBuffer,
	ReadBackBuffer,
};

enum class RegisterSlot {
	NONE = -1,
	CBVBegin = 0,	SRVBegin = 10,	UAVBegin = 20,
	CBV0 = 0,		SRV0 = 10,		UAV0 = 20,
	CBV1 = 1,		SRV1 = 11,		UAV1 = 21,
	CBV2 = 2,		SRV2 = 12,		UAV2 = 22,
	CBV3 = 3,		SRV3 = 13,		UAV3 = 23,
	CBV4 = 4,		SRV4 = 14,		UAV4 = 24,
	CBV5 = 5,		SRV5 = 15,		UAV5 = 25,
	CBV6 = 6,		SRV6 = 16,		UAV6 = 26,
	CBV7 = 7,		SRV7 = 17,		UAV7 = 27,
	CBV8 = 8,		SRV8 = 18,		UAV8 = 28,
	CBVEnd = 9,		SRVEnd = 19,	UAVEnd = 29,
};

enum class RegisterSpace {
	Space0 = 0, Space1 = 1, Space2 = 2,
	Space3 = 3, Space4 = 4, Space5 = 5,
	Space6 = 6, Space7 = 7, Space8 = 8,
};

constexpr static std::size_t kSwapChainBufferCount	= 2;
constexpr static std::size_t kFrameResourceCount = 3;
constexpr static std::size_t kMaxDescriptorTables = 32;
constexpr static std::size_t kDynamicDescriptorPerHeap = 32;
constexpr static std::size_t kDynamicDescriptorHeapCount = 2;
constexpr static std::size_t kVertexBufferSlotCount = 16;
constexpr static std::size_t kMaxRenderTargetCount = 8;

class FrameIndexProxy {
	static inline std::atomic_size_t _frameIndex = 0;
public:
	static const std::atomic_size_t &getConstantFrameIndexRef() noexcept {
		return _frameIndex;
	}
private:
	friend class FrameResourceQueue;
	static void startNewFrame() noexcept {
		_frameIndex = (_frameIndex + 1) % kFrameResourceCount;
	}
};

struct DeviceInitDesc;
class Adapter;
class CommandList;
class CommandQueue;
class ConstantBuffer;
class DefaultBuffer;
class DescriptorAllocation;
class DescriptorAllocatorPage;
class DescriptorAllocator;
class Device;
class FrameResourceItem;
class FrameResourceQueue;
class IndexBuffer;
class VertexBuffer;
class SwapChain;
class UploadBuffer;
class ResourceStateTracker;
class DynamicDescriptorHeap;
class RootSignatureDescHelper;
class RootSignature;
class PSO;
class GraphicsPSO;
class ComputePSO;

interface IContext;
interface ICommonContext;
interface IGraphicsContext;
interface IComputeContext;

class SRStructuredBuffer;
class UAStructuredBuffer;
class ConsumeStructuredBuffer;
class AppendStructuredBuffer;
class ReadBackBuffer;

class DepthStencil2D;
class SamplerTexture2D;
class SamplerTexture2DArray;
class SamplerTextureCube;

class RenderTarget2D;
class RenderTarget2DArray;
class RenderTargetCube;

class UnorderedAccess2D;
class UnorderedAccess2DArray;
class UnorderedAccessCube;


interface IResource;
interface IShaderResource;
interface IShaderResource2D;
interface IShaderResource2DArray;
interface IShaderResourceCube;

interface IRenderTarget;
interface IRenderTarget2D;
interface IRenderTarget2DArray;
interface IRenderTargetCube;

interface IUnorderedAccess;
interface IUnorderedAccess2D;
interface IUnorderedAccess2DArray;
interface IUnorderedAccessCube;

interface IDepthStencil;
interface IDepthStencil2D;

interface IBufferResource;
interface IConstantBuffer;
interface IVertexBuffer;
interface IIndexBuffer;
interface IReadBackBuffer;

interface IStructuredBuffer;
interface IConsumeStructuredBuffer;
interface IAppendStructuredBuffer;

struct NonCopyable {
	NonCopyable() = default;
	NonCopyable(NonCopyable &&) = default;
	NonCopyable &operator=(NonCopyable &&) = default;
	virtual ~NonCopyable() = default;
	NonCopyable(const NonCopyable &) = delete;
	NonCopyable &operator=(const NonCopyable &) = delete;
};

struct RawData;

// frame resource constant buffer template
template<typename T = RawData>
class FRConstantBuffer;

using FRRawConstantBufferPtr = std::shared_ptr<FRConstantBuffer<RawData>>;

template<typename T>
using FRConstantBufferPtr = std::shared_ptr<FRConstantBuffer<T>>;


// frame resource structured buffer template
template<typename T = RawData>
class FRSRStructuredBuffer;

using FRRawStructuredBufferPtr = std::shared_ptr<FRSRStructuredBuffer<RawData>>;

template<typename T>
using FRStructuredBufferPtr = std::shared_ptr<FRSRStructuredBuffer<T>>;

}

using dx12lib::FRRawConstantBufferPtr;
using dx12lib::FRConstantBufferPtr;
using dx12lib::FRRawStructuredBufferPtr;
using dx12lib::FRStructuredBufferPtr;