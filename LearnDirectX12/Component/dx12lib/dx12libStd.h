#pragma once
#define NOMINMAX
#include <cassert>
#include <cstdint>
#include <wrl.h>
#include <exception>
#include "D3Dx12.h"
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

enum AttachmentPoint : std::size_t {
	Color0,
	Color1,
	Color2,
	Color3,
	Color4,
	Color5,
	Color6,
	Color7,
	NumAttachmentPoints,
};

enum class ResourceType {
	Unknown				  = ((0x0) << 0),
	VertexBuffer		  = ((0x1) << 0),
	IndexBuffer			  = ((0x1) << 1),
	ConstantBuffer		  = ((0x1) << 2),
	DepthStencilBuffer	  = ((0x1) << 3),
	RenderTargetBuffer	  = ((0x1) << 4),
	ShaderResourceBuffer  = ((0x1) << 5),
	UnorderedAccessBuffer = ((0x1) << 6),
	ReadBackBuffer		  = ((0x1) << 7),
	StructuredBuffer	  = ((0x1) << 8),
};

ResourceType operator|(const ResourceType &lhs, const ResourceType &rhs);
bool operator&(const ResourceType &lhs, const ResourceType &rhs);

constexpr static std::size_t kSwapChainBufferCount = 2;
constexpr static std::size_t kFrameResourceCount = 3;
constexpr static std::size_t kMaxDescriptorTables = 32;
constexpr static std::size_t kDynamicDescriptorPerHeap = 32;
constexpr static std::size_t kDynamicDescriptorHeapCount = 2;
constexpr static std::size_t kVertexBufferSlotCount = 16;

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
class RenderTarget;
class SwapChain;
class UploadBuffer;
class ResourceStateTracker;
class IResource;
class DynamicDescriptorHeap;
class RootSignatureDescHelper;
class RootSignature;
class PSO;
class GraphicsPSO;
class ComputePSO;
class CommonContext;
class GraphicsContext;
class ComputeContext;
class StructuredBuffer;
class UnorderedAccessBuffer;
class ReadBackBuffer;
class DepthStencilBuffer;
class RenderTargetBuffer;
class ShaderResourceBuffer;
class IShaderSourceResource;
class IResource;

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
class FRStructuredBuffer;

using FRRawStructuredBufferPtr = std::shared_ptr<FRStructuredBuffer<RawData>>;

template<typename T>
using FRStructuredBufferPtr = std::shared_ptr<FRStructuredBuffer<T>>;

}

using dx12lib::FRRawConstantBufferPtr;
using dx12lib::FRConstantBufferPtr;
using dx12lib::FRRawStructuredBufferPtr;
using dx12lib::FRStructuredBufferPtr;