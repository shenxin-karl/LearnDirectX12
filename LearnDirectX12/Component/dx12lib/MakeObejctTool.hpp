#pragma once
#include "dx12libStd.h"
#include <type_traits>

namespace dx12lib {

template<typename T>
class _MakeObjectTool : public T {
public:
	template<typename... Args>
	_MakeObjectTool(Args&&... args) : T(std::forward<Args>(args)...) {
	}
};

using MakeCommandList = _MakeObjectTool<CommandList>;
using MakeCommandListProxy = _MakeObjectTool<CommandListProxy>;
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
using MakeRenderTargetBuffer = _MakeObjectTool<RenderTargetBuffer>;
using MakeDepthStencilBuffer = _MakeObjectTool<DepthStencilBuffer>;
using MakeUnorderedAccessBuffer = _MakeObjectTool<UnorderedAccessBuffer>;
using MakeStructedBuffer = _MakeObjectTool<StructuredBuffer>;
using MakeReadbackBuffer = _MakeObjectTool<ReadbackBuffer>;
using MakeShaderResourceBuffer = _MakeObjectTool<ShaderResourceBuffer>;


}