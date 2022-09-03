#include "SobelFilter.h"
#include "D3D/d3dutil.h"
#include "D3D/Shader/D3DShaderResource.h"
#include <dx12lib/Resource/ResourceStd.h>
#include <dx12lib/Texture/TextureStd.h>
#include <dx12lib/Pipeline/PipelineStd.h>
#include <dx12lib/Device/Device.h>

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

std::shared_ptr<dx12lib::UnorderedAccess2D> SobelFilter::getOutput() const {
	return _pSobelMap;
}

void SobelFilter::tryBuildSobelMap(dx12lib::ComputeContextProxy pComputeList, DXGI_FORMAT format) {
	if (_pSobelMap == nullptr || _pSobelMap->getWidth() != _width ||
		_pSobelMap->getHeight() != _height || _pSobelMap->getFormat() != format) 
	{
		_pSobelMap = pComputeList->createUnorderedAccess2D(_width, _height, format);
	}
}

void SobelFilter::tryBuildRootSignature(dx12lib::ComputeContextProxy pComputeList) {
	if (_pRootSignature != nullptr)
		return;

	auto pSharedDevice = pComputeList->getDevice().lock();
	_pRootSignature = pSharedDevice->createRootSignature(1);
	_pRootSignature->at(0).initAsDescriptorTable({
		{ dx12lib::RegisterSlot::SRV0, 1 },
		{ dx12lib::RegisterSlot::UAV0, 1 },
	});
	_pRootSignature->finalize();
}

void SobelFilter::tryBuildProducePSO(dx12lib::ComputeContextProxy pComputeList) {
	if (_pProducePSO != nullptr)
		return;

	tryBuildRootSignature(pComputeList);
	auto pSharedDevice = pComputeList->getDevice().lock();
	_pProducePSO = pSharedDevice->createComputePSO("SobelProducePSO");
	auto sobelFilterCSHlsl = getD3DResource("shader/SobelFilterCS.hlsl");

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
	if (_pApplyPSO != nullptr)
		return;

	tryBuildRootSignature(pComputeList);
	auto pSharedDevice = pComputeList->getDevice().lock();
	_pApplyPSO = pSharedDevice->createComputePSO("SobelProducePSO");
	auto sobelFilterCSHlsl = getD3DResource("HlslShader/SobelFilterCS.hlsl");

	D3D_SHADER_MACRO macros[] = { { "APPLY_MODE", "" }, { nullptr, nullptr } };
	_pApplyPSO->setRootSignature(_pRootSignature);
	_pApplyPSO->setComputeShader(compileShader(
		sobelFilterCSHlsl.begin(),
		sobelFilterCSHlsl.size(),
		macros,
		"SobelApply",
		"cs_5_0"
	));
	_pApplyPSO->finalize();
}

void SobelFilter::produceImpl(dx12lib::ComputeContextProxy pComputeList, 
	std::shared_ptr<dx12lib::ITextureResource2D> pInput)
{
	assert(pInput != nullptr);
	tryBuildSobelMap(pComputeList, DXGI_FORMAT_R8_UNORM);
	tryBuildProducePSO(pComputeList);

	pComputeList->transitionBarrier(pInput, D3D12_RESOURCE_STATE_GENERIC_READ);
	pComputeList->transitionBarrier(_pSobelMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	pComputeList->setComputePSO(_pProducePSO);
	pComputeList->setShaderResourceView(dx12lib::RegisterSlot::SRV0, pInput->getSRV());
	pComputeList->setUnorderedAccessView(dx12lib::RegisterSlot::UAV0, _pSobelMap->getUAV());
	uint32 numXGroup = static_cast<uint32>(std::ceil(float(_width) / kMaxSobelThreadCount));
	uint32 numYGroup = static_cast<uint32>(std::ceil(float(_height) / kMaxSobelThreadCount));
	pComputeList->dispatch(numXGroup, numYGroup);
}

void SobelFilter::applyImpl(dx12lib::ComputeContextProxy pComputeList, 
	std::shared_ptr<dx12lib::ITextureResource2D> pInput)
{
	assert(pInput != nullptr);
	tryBuildSobelMap(pComputeList, pInput->getFormat());
	tryBuildApplyPSO(pComputeList);

	pComputeList->transitionBarrier(pInput, D3D12_RESOURCE_STATE_GENERIC_READ);
	pComputeList->transitionBarrier(_pSobelMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	pComputeList->setComputePSO(_pApplyPSO);
	pComputeList->setShaderResourceView(dx12lib::RegisterSlot::SRV0, pInput->getSRV());
	pComputeList->setUnorderedAccessView(dx12lib::RegisterSlot::UAV0, _pSobelMap->getUAV());
	uint32 numXGroup = static_cast<uint32>(std::ceil(float(_width) / kMaxSobelThreadCount));
	uint32 numYGroup = static_cast<uint32>(std::ceil(float(_height) / kMaxSobelThreadCount));
	pComputeList->dispatch(numXGroup, numYGroup);
}

}