#include "CommandList.h"

namespace dx12lib {

ID3D12GraphicsCommandList * CommandList::getD3DCommandList() const noexcept {
	return _pCommandList.Get();
}

HRESULT CommandList::close() {
	return _pCommandList->Close();
}

}