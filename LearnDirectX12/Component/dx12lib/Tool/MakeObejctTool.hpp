#pragma once
#include <dx12lib/dx12libStd.h>
#include <type_traits>

namespace dx12libTool {

using namespace dx12lib;

template<typename T>
class _MakeObjectTool : public T {
public:
	template<typename... Args>
	_MakeObjectTool(Args&&... args) : T(std::forward<Args>(args)...) {
	}
};

using MakeCommandList = _MakeObjectTool<CommandList>;
using MakeCommandQueue = _MakeObjectTool<CommandQueue>;
using MakeConstantBuffer = _MakeObjectTool<ConstantBuffer>;
using MakeDescriptorAllocation = _MakeObjectTool<DescriptorAllocation>;
using MakeDescriptorAllocatorPage= _MakeObjectTool<DescriptorAllocatorPage>;
using MakeDescriptorAllocator = _MakeObjectTool<DescriptorAllocator>;
using MakeFrameResourceItem = _MakeObjectTool<FrameResourceItem>;
using MakeFrameResourceQueue = _MakeObjectTool<FrameResourceQueue>;
using MakeRenderTarget = _MakeObjectTool<RenderTarget>;
using MakeSwapChain = _MakeObjectTool<SwapChain>;
using MakeResourceStateTracker = _MakeObjectTool<ResourceStateTracker>;
using MakeDynamicDescriptorHeap = _MakeObjectTool<DynamicDescriptorHeap>;
using MakeRootSignature = _MakeObjectTool<RootSignature>;
using MakeGraphicsPSO = _MakeObjectTool<GraphicsPSO>;
using MakeComputePSO = _MakeObjectTool<ComputePSO>;

using MakeIndexBuffer = _MakeObjectTool<IndexBuffer>;
using MakeVertexBuffer = _MakeObjectTool<VertexBuffer>;
using MakeRenderTargetBuffer = _MakeObjectTool<RenderTarget2D>;
using MakeUnorderedAccessBuffer = _MakeObjectTool<UnorderedAccessBuffer>;
using MakeStructuredBuffer = _MakeObjectTool<StructuredBuffer>;
using MakeReadBackBuffer = _MakeObjectTool<ReadBackBuffer>;

using MakeTexture2D = _MakeObjectTool<Texture2D>;
using MakeTexture2DArray = _MakeObjectTool<Texture2DArray>;
using MakeTextureCube = _MakeObjectTool<TextureCube>;

using MakeRenderTarget2D = _MakeObjectTool<RenderTarget2D>;
using MakeRenderTarget2DArray = _MakeObjectTool<RenderTarget2DArray>;
using MakeRenderTargetCube = _MakeObjectTool<RenderTargetCube>;

using MakeDepthStencil2D = _MakeObjectTool<DepthStencil2D>;

using MakeUnorderedAccess2D = _MakeObjectTool<UnorderedAccess2D>;
using MakeUnorderedAccess2DArray = _MakeObjectTool<UnorderedAccess2DArray>;
using MakeUnorderedAccessCube = _MakeObjectTool<UnorderedAccessCube>;

template<typename T>
using MakeFRConstantBuffer = _MakeObjectTool<FRConstantBuffer<T>>;

using MakeFRRawConstantBuffer = _MakeObjectTool<FRConstantBuffer<RawData>>;

using MakeFRRawStructuredBuffer = _MakeObjectTool<FRStructuredBuffer<RawData>>;

template<typename T>
using MakeFRStructuredBuffer = _MakeObjectTool<FRStructuredBuffer<T>>;

}