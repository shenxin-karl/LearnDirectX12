#pragma once
#include "dx12libStd.h"

namespace dx12lib {

class PipelineStateObject {
public:
	PipelineStateObject(std::weak_ptr<Device> pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC &desc);
	PipelineStateObject(const PipelineStateObject &) = delete;
	WRL::ComPtr<ID3D12PipelineState> getPSO() const;
private:
	WRL::ComPtr<ID3D12PipelineState> _pPso;
};

}