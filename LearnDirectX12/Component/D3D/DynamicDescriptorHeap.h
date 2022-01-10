#pragma once
#include "d3dutil.h"
#include <functional>
#include <queue>
#include <bitset>

namespace d3dUtil {

class RootSignature;
class CommandList;

class DynamicDescriptorHeap {
public:
	DynamicDescriptorHeap(ID3D12Device *pDevice, 
		D3D12_DESCRIPTOR_HEAP_TYPE heapType, 
		uint32 numDescriptorPreHeap = 1024
	);

	~DynamicDescriptorHeap() = default;

	void stageDescriptors(uint32 rootParameterIndex, 
		uint32 offset, 
		uint32 numDescriptors, 
		const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors
	);


	void commitStagedDescriptorsForDraw(CommandList &commandList);
	void commitStagedDescriptorsForDispatch(CommandList &commandList);
	auto copyDescriptor(CommandList &commandList, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor);
	void parseRootSignature(const RootSignature &rootSignature);	
	void reset();
private:
	WRL::ComPtr<ID3D12DescriptorHeap> requestDescriptorHeap();
	WRL::ComPtr<ID3D12DescriptorHeap> createDescriptorHeap();
	uint32 computeStaleDescriptorCount() const noexcept;

	// Can be the following values:
	// ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable
	// ID3D12GraphicsCommandList::SetComputeRootDescriptorTable
	using StageFunc = std::function<void(ID3D12GraphicsCommandList *, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)>;
	void commitStagedDescriptors(CommandList &commandList, const StageFunc &setFunc);

	static constexpr uint32 kMaxDescriptorTables = 32;

	struct DescriptorTableCache {
		void reset();
		uint32 numDescriptors = 0;
		D3D12_CPU_DESCRIPTOR_HANDLE *pBaseDescriptor = nullptr;
	};
private:
	ID3D12Device  *_pDevice;
	D3D12_DESCRIPTOR_HEAP_TYPE  _heapType;
	uint32  _numDescriptorPerHeap;
	uint32  _descriptorHandleIncrementSize;
	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> _pDescriptorHandleCache;
	DescriptorTableCache  _descriptorTableCache[kMaxDescriptorTables];
	std::bitset<32>  _descriptorTableBitMask;
	std::bitset<32>  _staleDescriptorTableBitMask;

	using DescriptorHeapPool = std::queue<WRL::ComPtr<ID3D12DescriptorHeap>>;
	DescriptorHeapPool  _descriptorHeapPool;
	DescriptorHeapPool  _availableDescriptorHeaps;

	WRL::ComPtr<ID3D12DescriptorHeap>  _pCurrentDescriptorHeap;	// bound to command list
	CD3DX12_GPU_DESCRIPTOR_HANDLE  _currentGPUDescriptorHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE  _currentCPUDescriptorHandle;
	uint32  _numFreeHandles;
};

}