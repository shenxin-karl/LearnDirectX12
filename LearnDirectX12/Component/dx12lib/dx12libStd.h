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

enum class CommandQueueType {
	None = -1,
	Direct,
	Compute,
	Copy,
	NumType,
};

enum AttachmentPoint : std::size_t {
	Color0,
	Color1,
	Color2,
	Color3,
	Color4,
	Color5,
	Color6,
	Color7,
	DepthStencil,
	NumAttachmentPoints,
};

CommandQueueType toCommandQueueType(D3D12_COMMAND_LIST_TYPE type);

constexpr static std::size_t kComandQueueTypeCount = 3;
constexpr static std::size_t kSwapChainBufferCount = 2;
constexpr static std::size_t kFrameResourceCount = 3;
constexpr static std::size_t kMaxDescriptorTables = 32;
constexpr static std::size_t kDynamicDescriptorPerHeap = 32;
constexpr static std::size_t kDynamicDescriptorHeapCount = 2;
constexpr static std::size_t kVertexBufferSlotCount = 16;

struct DeviceInitDesc;

class Adapter;
class CommandList;
class CommandListProxy;
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
class Texture;
class UploadBuffer;
class ResourceStateTracker;
class IResource;
class DynamicDescriptorHeap;
class RootSignatureDescHelper;
class RootSignature;
class PSO;
class GraphicsPSO;
class ComputePSO;

}