#include "PipelineStateObject.h"
#include "Device.h"

namespace dx12lib {

PipelineStateObject::PipelineStateObject(std::weak_ptr<Device> pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC &desc)
{
	ThrowIfFailed(pDevice.lock()->getD3DDevice()->CreateGraphicsPipelineState(
		&desc,
		IID_PPV_ARGS(&_pPso)
	));
}

WRL::ComPtr<ID3D12PipelineState> PipelineStateObject::getPSO() const {
	return _pPso;
}

}