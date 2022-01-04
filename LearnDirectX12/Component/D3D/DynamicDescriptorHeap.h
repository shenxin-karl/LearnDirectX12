#pragma once
#include "d3dutil.h"
#include <functional>

namespace d3dUtil {

class RootSignature;
class CommandList;

class DynamicDescriptorHeap {
public:
	DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32 numDescriptorPreHeap = 1024);
	virtual ~DynamicDescriptorHeap();
	void stageDescriptors(uint32 rootParameterIndex, 
		uint32 offset, 
		uint32 numDescriptors, 
		const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors
	);
	using StageFunc = std::function<void(ID3D12GraphicsCommandList *, UINT, D3D12_CPU_DESCRIPTOR_HANDLE)>;
	void commitStagedDescriptors(CommandList &commandList, const StageFunc &setFunc);
	void commitStagedDescriptorsForDraw(CommandList &commandList);
	void commitStagedDescriptorsForDispatch(CommandList &commandList);
	auto copyDescriptor(CommandList &commandList, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor);
	void parseRootSignature(const RootSignature &rootSignature);	
	void reset();
private:
	WRL::ComPtr<ID3D12DescriptorHeap> requestDescriptorHeap();
	WRL::ComPtr<ID3D12DescriptorHeap> createDescriptorHeap();
	uint32 computeStaleDescriptorCount() const noexcept;
	static inline uint32 kMaxDescriptorTables = 32;

	struct DescriptorTableCache {
		void reset();
		uint32 numDescriptors = 0;
		D3D12_CPU_DESCRIPTOR_HANDLE *pBaseDescriptor = nullptr;
	};
};

}