#include "RootSignature.h"
#include "DynamicDescriptorHeap.h"
#include "Device.h"
#include "CommandList.h"

namespace dx12lib {

DynamicDescriptorHeap::DynamicDescriptorHeap(std::weak_ptr<Device> pDevice, 
	D3D12_DESCRIPTOR_HEAP_TYPE heapType, 
	size_t numDescriptorsPerHeap)
: _numDescriptorsPerHeap(numDescriptorsPerHeap), _heapType(heapType), _pDevice(pDevice)
{
	_descriptorHandleIncrementSize = pDevice.lock()->getD3DDevice()->GetDescriptorHandleIncrementSize(heapType);
	reset();
}

void DynamicDescriptorHeap::parseRootSignature(std::shared_ptr<RootSignature> pRootSignature) {
	assert(pRootSignature != nullptr);

	size_t currentOffset = 0;
	const auto &rootSignatureDesc = pRootSignature->getRootSignatureDesc();
	const auto descriptorTableBitMask = pRootSignature->getDescriptorTableBitMask(_heapType);
	for (std::size_t i = 0; i < kMaxDescriptorTables; ++i) {
		if (!descriptorTableBitMask.test(i))
			continue;

		size_t numDescriptors = pRootSignature->getNumDescriptorsByType(_heapType, i);
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
	size_t offset,
	size_t numDescripotrs,
	const D3D12_CPU_DESCRIPTOR_HANDLE &srcDescriptor)
{
	if (numDescripotrs > _numDescriptorsPerHeap || rootParameterIndex >= kMaxDescriptorTables)
		throw std::bad_alloc();

	auto &descriptorTableCache = _descriptorTableCache[rootParameterIndex];
	if ((offset + numDescripotrs) > descriptorTableCache._numDescriptors) {
		std::stringstream sbuf;
		sbuf << "Number of descriptors exceeds the number of descriptors in the descriptor table." << std::endl;
		sbuf << "rootParameterIndex: " << rootParameterIndex << std::endl
			<< "offset: " << offset << std::endl
			<< "numDescriptors: " << numDescripotrs << std::endl
			<< "descriptorTableCache._numDescriptors: " << descriptorTableCache._numDescriptors << std::endl;
		throw std::length_error(sbuf.str());
	}

	D3D12_CPU_DESCRIPTOR_HANDLE *pDstDescriptor = (descriptorTableCache._pBaseHandle + offset);
	bool dirty = false;
	for (size_t i = 0; i < numDescripotrs; ++i) {
		auto descriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			srcDescriptor, 
			static_cast<INT>(i), 
			static_cast<UINT>(_descriptorHandleIncrementSize)
		);
		if (descriptor.ptr != pDstDescriptor[i].ptr) {
			pDstDescriptor[i] = descriptor;
			dirty = true;
		}
	}
	if (dirty)
		_staleDescriptorTableBitMask.set(rootParameterIndex, true);
}

size_t DynamicDescriptorHeap::computeStaleDescriptorCount() const {
	if (_staleDescriptorTableBitMask.none())
		return 0;

	size_t numStaleDescriptors = 0;
	for (std::size_t i = 0; i < kMaxDescriptorTables; ++i) {
		if (_staleDescriptorTableBitMask.test(i))
			numStaleDescriptors += _descriptorTableCache[i]._numDescriptors;
	}
	return numStaleDescriptors;
}

void DynamicDescriptorHeap::commitDescriptorTables(std::shared_ptr<CommandList> pCmdList, const CommitFunc &setFunc) {
	size_t numDescriptors = computeStaleDescriptorCount();
	if (numDescriptors == 0)
		return;

	auto *pD3DCommandList = pCmdList->getD3DCommandList();
	if (_pCurrentDescriptorHeap == nullptr || _numFreeHandles < numDescriptors) {
		_staleDescriptorTableBitMask = _descriptorTableBitMask;
		_pCurrentDescriptorHeap = requestDescriptorHeap();
		_currentCPUDescriptorHandle = _pCurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		_currentGPUDescriptorHandle = _pCurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		_numFreeHandles = _numDescriptorsPerHeap;
		pCmdList->setDescriptorHeap(_heapType, _pCurrentDescriptorHeap.Get());
	}

	auto *pD3DDevice = _pDevice.lock()->getD3DDevice();
	for (std::size_t rootIndex = 0; rootIndex < kMaxDescriptorTables; ++rootIndex) {
		if (!_staleDescriptorTableBitMask.test(rootIndex))
			continue;

		UINT numDescriptors = static_cast<UINT>(_descriptorTableCache[rootIndex]._numDescriptors);
		auto *pSrcHandle = _descriptorTableCache[rootIndex]._pBaseHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE pDstDescriptorRangeStarts[] = { _currentCPUDescriptorHandle };
		UINT pDstDescriptorRangeSizes[] = { numDescriptors };
		pD3DDevice->CopyDescriptors(
			1, pDstDescriptorRangeStarts, pDstDescriptorRangeSizes, 
			numDescriptors, pSrcHandle, nullptr,
			_heapType
		);

		// Bind to the Command list 
		setFunc(pD3DCommandList, static_cast<UINT>(rootIndex), _currentGPUDescriptorHandle);
		_numFreeHandles -= numDescriptors;
		_currentCPUDescriptorHandle.Offset(static_cast<INT>(numDescriptors), static_cast<UINT>(_descriptorHandleIncrementSize));
		_currentGPUDescriptorHandle.Offset(static_cast<INT>(numDescriptors), static_cast<UINT>(_descriptorHandleIncrementSize));
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
		heapDesc.NumDescriptors = static_cast<UINT>(_numDescriptorsPerHeap);
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