#include "Device.h"
//#include "Resource.h"

namespace dx12lib {

//Resource::Resource(ID3D12Device *pDevice, const D3D12_RESOURCE_DESC &desc, const D3D12_CLEAR_VALUE *pClearValue /*= nullptr*/) {
//	ThrowIfFailed(pDevice->CreateCommittedResource(
//		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
//		D3D12_HEAP_FLAG_NONE,
//		&desc,
//		D3D12_RESOURCE_STATE_COMMON,
//		pClearValue,
//		IID_PPV_ARGS(&_pResource)
//	));
//}
//
//Resource::Resource(WRL::ComPtr<ID3D12Resource> pResource) : _pResource(pResource) {
//}
//
//WRL::ComPtr<ID3D12Resource> Resource::getD3DResource() const {
//	return _pResource;
//}

}