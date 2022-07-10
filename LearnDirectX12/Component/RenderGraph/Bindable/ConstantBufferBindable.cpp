#include "ConstantBufferBindable.h"

namespace rg {

ConstantBufferBindable::ConstantBufferBindable(const dx12lib::ShaderRegister &sr,
	std::shared_ptr<dx12lib::IConstantBuffer> pConstantBuffer)
: Bindable(BindableType::ConstantBuffer)
, _shaderRegister(sr)
, _pConstantBuffer(pConstantBuffer)
{
	assert(pConstantBuffer != nullptr);
}

void ConstantBufferBindable::bind(dx12lib::GraphicsContextProxy pGraphicsCtx) const {
	pGraphicsCtx->setConstantBuffer(_shaderRegister, _pConstantBuffer);
}

}
