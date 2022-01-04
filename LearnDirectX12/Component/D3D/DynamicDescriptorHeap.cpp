#include "DynamicDescriptorHeap.h"

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

void DynamicDescriptorHeap::parseRootSignature(const RootSignature &rootSignature) {
	_staleDescriptorTableBitMask = 0;
	const D3D12_ROOT_SIGNATURE_DESC &rootSinatureDesc = {};		// todo: 修改从 rootSignature 获取
	_descriptorTableBitMask = 0;								// todo: rootSignature 获取

}

}