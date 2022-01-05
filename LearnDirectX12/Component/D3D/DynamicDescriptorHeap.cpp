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

}