#include "ConstantBufferBindable.h"

namespace rg {

std::shared_ptr<ConstantBufferBindable> ConstantBufferBindable::make(const dx12lib::ShaderRegister &sr,
	std::shared_ptr<dx12lib::IConstantBuffer> pConstantBuffer, const std::source_location &sourceLocation)
{
	auto pBindable = std::make_shared<ConstantBufferBindable>(sourceLocation);
	pBindable->_pConstantBuffer = pConstantBuffer;
	pBindable->_shaderRegister = sr;
	return pBindable;
}

void ConstantBufferBindable::bind(dx12lib::IGraphicsContext &graphicsCtx) const {
	graphicsCtx.setConstantBuffer(_shaderRegister, _pConstantBuffer);
}

}
