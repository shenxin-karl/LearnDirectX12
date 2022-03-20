#include "BlurFilter.h"
#include "D3Dx12.h"
#include "dx12lib/IResource.h"

namespace GlobalShaderVar {
	
}

namespace d3d {

BlurFileter::BlurFileter(dx12lib::ComputeContextProxy pComputeContext,
	std::uint32_t width, 
	std::uint32_t height, 
	DXGI_FORMAT format) 
: _width(width), _height(height), _format(format)
{
	assert(format != DXGI_FORMAT_UNKNOWN);
	assert(width > 0);
	assert(height > 0);
	buildUnorderedAccessResouce(pComputeContext);
}

void BlurFileter::onResize(dx12lib::ComputeContextProxy pComputeList, std::uint32_t width, std::uint32_t height) {
	if (_width != width && _height != height) {
		_width = width;
		_height = height;
		buildUnorderedAccessResouce(pComputeList);
	}
}

void BlurFileter::produceImpl(dx12lib::ComputeContextProxy pComputeList,
	std::shared_ptr<dx12lib::IShaderSourceResource> pShaderResource,
	int blurCount,
	float sigma)
{
	assert(!pShaderResource->isShaderSample());

}

std::shared_ptr<dx12lib::UnorderedAccessBuffer> BlurFileter::getOuput() const {
	return _pBlurMap0;
}

void BlurFileter::buildUnorderedAccessResouce(dx12lib::ComputeContextProxy pComputeContext) {
	_pBlurMap0 = pComputeContext->createUnorderedAccessBuffer(_width, _height, _format);
	_pBlurMap1 = pComputeContext->createUnorderedAccessBuffer(_width, _height, _format);
}

}