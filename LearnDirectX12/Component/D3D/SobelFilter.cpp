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

void SobelFilter::tryBuildRootSignature(dx12lib::ComputeContextProxy pComputeList) {
	if (_pRootSignature != nullptr)
		return;

	dx12lib::RootSignatureDescHelper desc;
	desc.resize(2);
	desc[SR_Input].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	desc[UA_Output].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
	auto pSharedDevice = pComputeList->getDevice().lock();
	_pRootSignature = pSharedDevice->createRootSignature(desc);
}

void SobelFilter::tryBuildProducePSO(dx12lib::ComputeContextProxy pComputeList) {
	if (_pProducePSO != nullptr)
		return;

	tryBuildRootSignature(pComputeList);
	auto pSharedDevice = pComputeList->getDevice().lock();
	_pProducePSO = pSharedDevice->createComputePSO("SobelProducePSO");
	auto sobelFilterCSHlsl = getD3DShaderResource("shader/SobelFilterCS.hlsl");

	D3D_SHADER_MACRO macros[] = { { "PRODUCE_MODE", "" }, { nullptr, nullptr } };
	_pProducePSO->setRootSignature(_pRootSignature);
	_pProducePSO->setComputeShader(compileShader(
		sobelFilterCSHlsl.begin(),
		sobelFilterCSHlsl.size(),
		macros,
		"SobelProduce",
		"cs_5_0"
	));
	_pProducePSO->finalize();
}

void SobelFilter::tryBuildApplyPSO(dx12lib::ComputeContextProxy pComputeList) {
	if (_pAllpyPSO != nullptr)
		return;

	tryBuildRootSignature(pComputeList);
	auto pSharedDevice = pComputeList->getDevice().lock();
	_pAllpyPSO = pSharedDevice->createComputePSO("SobelProducePSO");
	auto sobelFilterCSHlsl = getD3DShaderResource("shader/SobelFilterCS.hlsl");

	D3D_SHADER_MACRO macros[] = { { "APPLY_MODE", "" }, { nullptr, nullptr } };
	_pAllpyPSO->setRootSignature(_pRootSignature);
	_pAllpyPSO->setComputeShader(compileShader(
		sobelFilterCSHlsl.begin(),
		sobelFilterCSHlsl.size(),
		macros,
		"SobelApply",
		"cs_5_0"
	));
	_pAllpyPSO->finalize();
}

void SobelFilter::produceImpl(dx12lib::ComputeContextProxy pComputeList, 
	std::shared_ptr<dx12lib::IShaderSourceResource> pInput) 
{
	assert(pInput != nullptr);
	tryBuildSobelMap(pComputeList, DXGI_FORMAT_R8_UNORM);
	tryBuildProducePSO(pComputeList);

	pComputeList->transitionBarrier(pInput, D3D12_RESOURCE_STATE_GENERIC_READ);
	pComputeList->transitionBarrier(_pSobelMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	pComputeList->setComputePSO(_pProducePSO);
	pComputeList->setShaderResourceBuffer(pInput, SR_Input);
	pComputeList->setUnorderedAccessBuffer(_pSobelMap, UA_Output);
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

	pComputeList->setComputePSO(_pAllpyPSO);
	pComputeList->setShaderResourceBuffer(pInput, SR_Input);
	pComputeList->setUnorderedAccessBuffer(_pSobelMap, UA_Output);
	uint32 numXGroup = static_cast<uint32>(std::ceil(float(_width) / kMaxSobelThreadCount));
	uint32 numYGroup = static_cast<uint32>(std::ceil(float(_height) / kMaxSobelThreadCount));
	pComputeList->dispatch(numXGroup, numYGroup);
}

}