#include "RootSignature.h"
#include "DynamicDescriptorHeap.h"
#include "Device.h"
#include "CommandList.h"

namespace dx12lib {

DynamicDescriptorHeap::DynamicDescriptorHeap(std::weak_ptr<Device> pDevice, 
	D3D12_DESCRIPTOR_HEAP_TYPE heapType, 
	uint32 numDescriptorsPerHeap) 
: _pDevice(pDevice), _numDescriptorsPerHeap(numDescriptorsPerHeap), _heapType(heapType)
{
	_descriptorHandleIncrementSize = pDevice.lock()->getD3DDevice()->GetDescriptorHandleIncrementSize(heapType);
	reset();
}

void DynamicDescriptorHeap::parseRootSignature(std::shared_ptr<RootSignature> pRootSignature) {
	assert(pRootSignature == nullptr);
	reset();

	uint32 currentOffset = 0;
	const auto &rootSignatureDesc = pRootSignature->getRootSignatureDesc();
	const auto descriptorTableBitMask = pRootSignature->getDescriptorTableBitMask(_heapType);
	for (std::size_t i = 0; i < kMaxDescriptorTables; ++i) {
		if (!descriptorTableBitMask.test(i))
			continue;
		
		uint32 numDescriptors = pRootSignature->getNumDescriptorsByType(_heapType, i);
		DescriptorTableCache &descriptorTableCache = _descriptorTableCache[i];
		descriptorTableCache._numDescriptors = numDescriptors;
		descriptorTableCache._pBaseHandle = _descriptorHandleCache.data() + currentOffset;
		currentOffset += numDescriptors;
		_descriptorTableBitMask.set(i);
	}
	/// out of cache range
	assert(currentOffset < kMaxDescriptorTables);
}

void DynamicDescriptorHeap::commitStagedDescriptorForDraw(std::shared_ptr<CommandList> pCmdList) {
	commitDescriptorTables(pCmdList, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
}

void DynamicDescriptorHeap::commitStagedDescriptorForDispatch(std::shared_ptr<CommandList> pCmdList) {
	commitDescriptorTables(pCmdList, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
}

void DynamicDescriptorHeap::reset() {
	_numFreeHandles = 0;
	_pCurrentDescriptorHeap = nullptr;
	_currentCPUDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
	_currentGPUDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
	_descriptorTableBitMask.reset();
	_staleDescriptorTableBitMask.reset();
	_availableDescriptorHeaps = _descriptorHeapPool;
	for (std::size_t i = 0; i < kMaxDescriptorTables; ++i) {
		_descriptorTableCache[i].reset();
		_descriptorHandleCache[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
	}
}

void DynamicDescriptorHeap::stageDescriptors(size_t rootParameterIndex, 
	uint32 offset, 
	uint32 numDescripotrs, 
	const D3D12_CPU_DESCRIPTOR_HANDLE &srcDescriptor) 
{
	if (numDescripotrs > _numDescriptorsPerHeap || rootParameterIndex >= kMaxDescriptorTables)
		throw std::bad_alloc();

	auto &descriptorTableCache = _descriptorTableCache[rootParameterIndex];
	if ((offset + numDescripotrs) > descriptorTableCache._numDescriptors)
		throw std::length_error("Number of descriptors exceeds the number of descriptors in the descriptor table.");

	D3D12_CPU_DESCRIPTOR_HANDLE *pDstDescriptor = (descriptorTableCache._pBaseHandle + offset);
	for (uint32 i = 0; i < numDescripotrs; ++i)
		pDstDescriptor[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(srcDescriptor, i, _descriptorHandleIncrementSize);

	_staleDescriptorTableBitMask.set(rootParameterIndex, true);
}

uint32 DynamicDescriptorHeap::computeStaleDescriptorCount() const {
	uint32 numStaleDescriptors = 0;
	for (std::size_t i = 0; i < kMaxDescriptorTables; ++i) {
		if (_staleDescriptorTableBitMask.test(i))
			numStaleDescriptors += _descriptorTableCache[i]._numDescriptors;
	}
	return numStaleDescriptors;
}

void DynamicDescriptorHeap::commitDescriptorTables(std::shared_ptr<CommandList> pCmdList, const CommitFunc &setFunc) {
	uint32 numDescriptors = computeStaleDescriptorCount();
	if (numDescriptors == 0)
		return;

	if (_pCurrentDescriptorHeap == nullptr || _numFreeHandles < numDescriptors) {
		_staleDescriptorTableBitMask = _descriptorTableBitMask;
		_pCurrentDescriptorHeap = requestDescriptorHeap();
		_currentCPUDescriptorHandle = _pCurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		_currentGPUDescriptorHandle = _pCurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		_numFreeHandles = _numDescriptorsPerHeap;
	}

	auto *pD3DDevice = _pDevice.lock()->getD3DDevice();
	for (std::size_t i = 0; i < kMaxDescriptorTables; ++i) {
		if (!_staleDescriptorTableBitMask.test(i))
			continue;

		UINT numDescriptors = _descriptorTableCache[i]._numDescriptors;
		auto *pSrcHandle = _descriptorTableCache[i]._pBaseHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE pDstDescriptorRangeStarts[] = { _currentCPUDescriptorHandle };
		UINT pDstDescriptorRangeSizes[] = { numDescriptors };
		pD3DDevice->CopyDescriptors(
			1, pDstDescriptorRangeStarts, pDstDescriptorRangeSizes, 
			1, pSrcHandle, nullptr, 
			_heapType
		);

		setFunc(pCmdList->getD3DCommandList(), numDescriptors, _currentGPUDescriptorHandle);
		_numFreeHandles -= numDescriptors;
		_currentCPUDescriptorHandle.Offset(numDescriptors, _descriptorHandleIncrementSize);
		_currentGPUDescriptorHandle.Offset(numDescriptors, _descriptorHandleIncrementSize);
	}
	_staleDescriptorTableBitMask.reset();
}

WRL::ComPtr<ID3D12DescriptorHeap> DynamicDescriptorHeap::requestDescriptorHeap() {
	WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
	if (!_availableDescriptorHeaps.empty()) {
		pDescriptorHeap = _availableDescriptorHeaps.front();
		_availableDescriptorHeaps.pop();
	} else {
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.NodeMask = 0;
		heapDesc.NumDescriptors = _numDescriptorsPerHeap;
		heapDesc.Type = _heapType;
		ThrowIfFailed(_pDevice.lock()->getD3DDevice()->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(&pDescriptorHeap)
		));
		_descriptorHeapPool.push(pDescriptorHeap);
	}
	return pDescriptorHeap;
}

DynamicDescriptorHeap::DescriptorTableCache::DescriptorTableCache() {
	reset();
}

void DynamicDescriptorHeap::DescriptorTableCache::reset() {
	_numDescriptors = 0;
	_pBaseHandle = nullptr;
}

}