#pragma once
#include "dx12libStd.h"

namespace dx12lib {

class DynamicDescriptorHeap {
public:
	DynamicDescriptorHeap(std::weak_ptr<Device> pDevice, 
		D3D12_DESCRIPTOR_HEAP_TYPE heapType, 
		uint32 numDescriptorsPerHeap
	);
	void parseRootSignature(std::shared_ptr<RootSignature> pRootSignature);
	void commitStagedDescriptorForDraw(std::shared_ptr<CommandList> pCmdList);
	void commitStagedDescriptorForDispatch(std::shared_ptr<CommandList> pCmdList);
	void reset();
	void stageDescriptors(size_t rootParameterIndex, 
		uint32 offset, 
		uint32 numDescripotrs, 
		const D3D12_CPU_DESCRIPTOR_HANDLE &srcDescriptor
	);
private:
	uint32 computeStaleDescriptorCount() const;

	using CommitFunc = std::function<void(ID3D12GraphicsCommandList *, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)>;
	void commitDescriptorTables(std::shared_ptr<CommandList> pCmdList, const CommitFunc &setFunc);

	WRL::ComPtr<ID3D12DescriptorHeap> requestDescriptorHeap();

	using DescriptorHeapPool = std::queue<WRL::ComPtr<ID3D12DescriptorHeap>>;
	struct DescriptorTableCache {
		DescriptorTableCache();
		void reset();
	public:
		uint32                       _numDescriptors;
		D3D12_CPU_DESCRIPTOR_HANDLE *_pBaseHandle;
	};

	using DescriptorHandleCache = std::array<D3D12_CPU_DESCRIPTOR_HANDLE, kMaxDescriptorTables>;
private:
	uint32                            _numDescriptorsPerHeap;
	uint32                            _descriptorHandleIncrementSize;
	D3D12_DESCRIPTOR_HEAP_TYPE        _heapType;
	DescriptorTableCache              _descriptorTableCache[kMaxDescriptorTables];
	std::weak_ptr<Device>             _pDevice;
	std::bitset<kMaxDescriptorTables> _descriptorTableBitMask;
	std::bitset<kMaxDescriptorTables> _staleDescriptorTableBitMask;
	DescriptorHandleCache             _descriptorHandleCache;
	DescriptorHeapPool                _descriptorHeapPool;
	DescriptorHeapPool                _availableDescriptorHeaps;

	uint32                            _numFreeHandles;
	CD3DX12_CPU_DESCRIPTOR_HANDLE     _currentCPUDescriptorHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE     _currentGPUDescriptorHandle;
	WRL::ComPtr<ID3D12DescriptorHeap> _pCurrentDescriptorHeap;
};

}