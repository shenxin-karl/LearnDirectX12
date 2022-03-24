#include "SobelFilter.h"
#include "D3Dx12.h"
#include "dx12lib/IResource.h"
#include "dx12lib/PipelineStateObject.h"
#include "dx12lib/RootSignature.h"
#include "dx12lib/Device.h"
#include "D3DShaderResource.h"
#include "d3dutil.h" 
#include "dx12lib/UnorderedAccessBuffer.h"
#include "dx12lib/ResourceStateTracker.h"

namespace d3d {

SobelFilter::SobelFilter(dx12lib::ComputeContextProxy pComputeContext, std::uint32_t width, std::uint32_t height)
: _width(width), _height(height) {
}

void SobelFilter::onResize(dx12lib::ComputeContextProxy pComputeList, std::uint32_t width, std::uint32_t height) {
	if (width != _width || height != _height) {
		_width = width;
		_height = height;
	}
}

std::shared_ptr<dx12lib::UnorderedAccessBuffer> SobelFilter::getOutput() const {
	return _pSobelMap;
}

void SobelFilter::tryBuildSobelMap(dx12lib::ComputeContextProxy pComputeList, DXGI_FORMAT format) {
	if (_pSobelMap == nullptr || _pSobelMap->getWidth() != _width ||
		_pSobelMap->getHeight() != _height || _pSobelMap->getFormat() != format) 
	{
		_pSobelMap = pComputeList->createUnorderedAccessBuffer(_width, _height, format);
	}
}

void SobelFilter::buildRootSignature(dx12lib::ComputeContextProxy pComputeList) {

}

void SobelFilter::tryBuildProducePSO(dx12lib::ComputeContextProxy pComputeList) {

}

void SobelFilter::tryBuildApplyPSO(dx12lib::ComputeContextProxy pComputeList) {

}

void SobelFilter::produceImpl(dx12lib::ComputeContextProxy pComputeList, 
	std::shared_ptr<dx12lib::IShaderSourceResource> pInput) 
{
	assert(pInput != nullptr);
	tryBuildSobelMap(pComputeList, DXGI_FORMAT_R8_UNORM);
	tryBuildProducePSO(pComputeList);

	pComputeList->transitionBarrier(pInput, D3D12_RESOURCE_STATE_GENERIC_READ);
	pComputeList->transitionBarrier(_pSobelMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	pComputeList->setPipelineStateObject(_pProducePSO);
	pComputeList->setShaderResourceBuffer(pInput, SR_Input);
	pComputeList->setShaderResourceBuffer(_pSobelMap, UA_Output);
	uint32 numXGroup = static_cast<uint32>(std::ceil(float(_width) / kMaxSobelThreadCount));
	uint32 numYGroup = static_cast<uint32>(std::ceil(float(_height) / kMaxSobelThreadCount));
	pComputeList->dispatch(numXGroup, numYGroup);
}

void SobelFilter::applyImpl(dx12lib::ComputeContextProxy pComputeList, 
	std::shared_ptr<dx12lib::IShaderSourceResource> pInput) 
{
	assert(pInput != nullptr);
	tryBuildSobelMap(pComputeList, pInput->getFormat());
	tryBuildApplyPSO(pComputeList);

	pComputeList->transitionBarrier(pInput, D3D12_RESOURCE_STATE_GENERIC_READ);
	pComputeList->transitionBarrier(_pSobelMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	pComputeList->setPipelineStateObject(_pAllpyPSO);
	pComputeList->setShaderResourceBuffer(pInput, SR_Input);
	pComputeList->setShaderResourceBuffer(_pSobelMap, UA_Output);
	uint32 numXGroup = static_cast<uint32>(std::ceil(float(_width) / kMaxSobelThreadCount));
	uint32 numYGroup = static_cast<uint32>(std::ceil(float(_height) / kMaxSobelThreadCount));
	pComputeList->dispatch(numXGroup, numYGroup);
}

}