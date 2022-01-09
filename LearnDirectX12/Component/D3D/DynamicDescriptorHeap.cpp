#include "DynamicDescriptorHeap.h"
#include <exception>
#include <stdexcept>

namespace d3dUtil {

DynamicDescriptorHeap::DynamicDescriptorHeap(ID3D12Device *pDevice,
	D3D12_DESCRIPTOR_HEAP_TYPE heapType, 
	uint32 numDescriptorPreHeap /*= 1024*/) 
: _pDevice(pDevice), _heapType(heapType), _numDescriptorPerHeap(numDescriptorPreHeap)
, _descriptorTableBitMask(0), _staleDescriptorTableBitMask(0)
, _currentCPUDescriptorHandle(D3D12_DEFAULT), _currentGPUDescriptorHandle(D3D12_DEFAULT)
, _numFreeHandles(0)
{
	_descriptorHandleIncrementSize = pDevice->GetDescriptorHandleIncrementSize(heapType);
	_pDescriptorHandleCache = std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(numDescriptorPreHeap);
}

void DynamicDescriptorHeap::stageDescriptors(uint32 rootParameterIndex, 
	uint32 offset, 
	uint32 numDescriptors, 
	const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors)
{
	if (numDescriptors > _numDescriptorPerHeap || rootParameterIndex > kMaxDescriptorTables)
		throw std::bad_alloc();

	auto &descriptorTableCache = _descriptorTableCache[rootParameterIndex];
	if (numDescriptors > descriptorTableCache.numDescriptors)
		throw std::length_error("Number of descriptors exceeds the number of descriptors in the descriptor table");

	auto *pDestDescriptor = descriptorTableCache.pBaseDescriptor + offset;
	for (std::size_t i = 0; i < static_cast<std::size_t>(numDescriptors); ++i)
		pDestDescriptor[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(srcDescriptors, i, _descriptorHandleIncrementSize);

	_staleDescriptorTableBitMask.set(rootParameterIndex);
}

void DynamicDescriptorHeap::commitStagedDescriptorsForDraw(CommandList &commandList) {
	commitStagedDescriptors(commandList, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
}

void DynamicDescriptorHeap::commitStagedDescriptorsForDispatch(CommandList &commandList) {
	commitStagedDescriptors(commandList, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
}

auto DynamicDescriptorHeap::copyDescriptor(CommandList &commandList, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor) {
	if (_pCurrentDescriptorHeap == nullptr || _numFreeHandles < 1) {
		_pCurrentDescriptorHeap = requestDescriptorHeap();
		_currentCPUDescriptorHandle = _pCurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		_currentGPUDescriptorHandle = _pCurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		_numFreeHandles = _numDescriptorPerHeap;
		//commandList.setDescriptorHeap();	// TODO 绑定到命令列表中
		_staleDescriptorTableBitMask = _descriptorTableBitMask;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE hGPU = _currentGPUDescriptorHandle;
	_pDevice->CopyDescriptorsSimple(1, _currentCPUDescriptorHandle, cpuDescriptor, _heapType);
	_currentCPUDescriptorHandle.Offset(1, _descriptorHandleIncrementSize);
	_currentGPUDescriptorHandle.Offset(1, _descriptorHandleIncrementSize);
	_numFreeHandles -= 1;
	return hGPU;
}

void DynamicDescriptorHeap::commitStagedDescriptors(CommandList &commandList, const StageFunc &setFunc) {
	uint32 numDescriptorToCommit = computeStaleDescriptorCount();
	if (numDescriptorToCommit > 0) {
		ID3D12GraphicsCommandList *pCmdList = nullptr;		// TODO get Command List By CommandList
		assert(pCmdList != nullptr);
		if (_pCurrentDescriptorHeap == nullptr || _numFreeHandles < numDescriptorToCommit) {
			_pCurrentDescriptorHeap = requestDescriptorHeap();
			_currentCPUDescriptorHandle = _pCurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			_currentGPUDescriptorHandle = _pCurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
			_numFreeHandles = _numDescriptorPerHeap;
			//commandList.setDescriptorHeap(_heapType, _pCurrentDescriptorHeap.Get());	// bing heap to command list
			_staleDescriptorTableBitMask = _descriptorTableBitMask;
		}

		for (std::size_t rootIndex = 0; rootIndex < _staleDescriptorTableBitMask.size(); ++rootIndex) {
			if (!_staleDescriptorTableBitMask.test(rootIndex))
				continue;

			auto &descripotrTableCache = _descriptorTableCache[rootIndex];
			UINT numSrcDescriptors = descripotrTableCache.numDescriptors;
			D3D12_CPU_DESCRIPTOR_HANDLE *pSrcDescriptorHandles = descripotrTableCache.pBaseDescriptor;

			D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRangeStarts[] = { _currentCPUDescriptorHandle };
			UINT destDescriptorRangeSizes[] = { numSrcDescriptors };
			_pDevice->CopyDescriptors(
				1, pDestDescriptorRangeStarts, destDescriptorRangeSizes,
				numSrcDescriptors, pSrcDescriptorHandles, nullptr,
				_heapType
			);
			setFunc(pCmdList, numSrcDescriptors, _currentGPUDescriptorHandle);
			_currentCPUDescriptorHandle.Offset(numSrcDescriptors, _descriptorHandleIncrementSize);
			_currentGPUDescriptorHandle.Offset(numSrcDescriptors, _descriptorHandleIncrementSize);
			_numFreeHandles -= numSrcDescriptors;
			_staleDescriptorTableBitMask.set(rootIndex, false);
		}
	}
}

void DynamicDescriptorHeap::parseRootSignature(const RootSignature &rootSignature) {
	_staleDescriptorTableBitMask = 0;
	const D3D12_ROOT_SIGNATURE_DESC &rootSinatureDesc = {};		// todo: 修改从 rootSignature 获取
	_descriptorTableBitMask = 0;								// todo: rootSignature 获取

	uint32 currentOffset = 0;
	for (std::size_t rootIndex = 0; rootIndex < 32; ++rootIndex) {
		if (_descriptorTableBitMask.test(rootIndex)) {
			uint32 numDescriptors = 0;// todo rootSignature.getNumDescriptors(rootIndex);
			DescriptorTableCache &descriptorTableCache = _descriptorTableCache[rootIndex];
			descriptorTableCache.numDescriptors = numDescriptors;
			descriptorTableCache.pBaseDescriptor = &_pDescriptorHandleCache[currentOffset];
			currentOffset += numDescriptors;
		}
	}
	
	//"The root signature requires more than the maximum number of descriptors per descriptor heap.
	// Consider increasing the maximum number of descriptors per descriptor heap."
	assert(currentOffset <= _numDescriptorPerHeap);
}

void DynamicDescriptorHeap::reset() {
	_availableDescriptorHeaps = _descriptorHeapPool;
	_pCurrentDescriptorHeap.Reset();
	_currentCPUDescriptorHandle = D3D12_CPU_DESCRIPTOR_HANDLE(0);
	_currentGPUDescriptorHandle = D3D12_GPU_DESCRIPTOR_HANDLE(0);
	_numFreeHandles = 0;
	for (auto &cache : _descriptorTableCache)
		cache.reset();
}

WRL::ComPtr<ID3D12DescriptorHeap> DynamicDescriptorHeap::requestDescriptorHeap() {
	WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
	if (!_availableDescriptorHeaps.empty()) {
		pDescriptorHeap = _availableDescriptorHeaps.front();
		_availableDescriptorHeaps.pop();
	} else {
		pDescriptorHeap = createDescriptorHeap();
		_descriptorHeapPool.push(pDescriptorHeap);
	}
	return pDescriptorHeap;
}

WRL::ComPtr<ID3D12DescriptorHeap> DynamicDescriptorHeap::createDescriptorHeap() {
	WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = _numDescriptorPerHeap;
	heapDesc.Type = _heapType;
	ThrowIfFailed(_pDevice->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(&pDescriptorHeap)
	));
	return pDescriptorHeap;
}

uint32 DynamicDescriptorHeap::computeStaleDescriptorCount() const noexcept {
	uint32 staleDescriptorCount = 0;
	for (std::size_t i = 0; i < _staleDescriptorTableBitMask.size(); ++i) {
		if (_staleDescriptorTableBitMask.test(i)) {
			auto &descriptorTableCache = _descriptorTableCache[i];
			staleDescriptorCount += descriptorTableCache.numDescriptors;
		}
	}
	return staleDescriptorCount;
}

void DynamicDescriptorHeap::DescriptorTableCache::reset() {
	numDescriptors = 0;
	pBaseDescriptor = nullptr;
}

}