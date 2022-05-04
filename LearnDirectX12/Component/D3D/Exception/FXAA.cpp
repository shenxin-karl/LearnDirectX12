#include "FXAA.h"
#include "D3D/d3dutil.h"
#include "D3D/Shader/D3DShaderResource.h"
#include "D3D/Shader/ShaderCommon.h"
#include <dx12lib/Device/DeviceStd.h>
#include <dx12lib/Pipeline/RootSignature.h>
#include <dx12lib/Pipeline/PipelineStateObject.h>
#include <dx12lib/Texture/TextureStd.h>

namespace d3d {

FXAA::FXAA(dx12lib::ComputeContextProxy pComputeCtx, 
	std::uint32_t width, 
	std::uint32_t height,
	DXGI_FORMAT format)
: _width(0), _height(0), _format(format)
{
	onResize(pComputeCtx, width, height);
}

void FXAA::_produceImpl(dx12lib::ComputeContextProxy pComputeCtx,
	std::shared_ptr<dx12lib::IShaderResource2D> pInput) const
{
	assert(pInput != nullptr);
	tryBuildConsolePSO(pComputeCtx->getDevice());

	pComputeCtx->transitionBarrier(pInput, D3D12_RESOURCE_STATE_GENERIC_READ);
	pComputeCtx->transitionBarrier(_pOutputMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	pComputeCtx->setComputePSO(_pConsolePSO);
	updateFXAASetting(pComputeCtx);
	pComputeCtx->setShaderResourceView(pInput->getSRV(), SR_Input);
	pComputeCtx->setUnorderedAccessView(_pOutputMap->getUAV(), UA_Output);
	std::size_t numXGroup = static_cast<std::size_t>(std::ceil(float(_width) / float(kFXAAThreadCount)));
	std::size_t numYGroup = static_cast<std::size_t>(std::ceil(float(_height) / float(kFXAAThreadCount)));
	pComputeCtx->dispatch(numXGroup, numYGroup);
}

void FXAA::onResize(dx12lib::ComputeContextProxy pComputeCtx, uint32 width, uint32 height) {
	if (_width != width || _height != height) {
		_width = width;
		_height = height;
		_pOutputMap = pComputeCtx->createUnorderedAccess2D(_width, _height, nullptr, _format);
	}
}

std::shared_ptr<dx12lib::UnorderedAccess2D> FXAA::getOutput() const {
	return _pOutputMap;
}

void FXAA::tryBuildRootSignature(std::weak_ptr<dx12lib::Device> pDevice) {
	if (_pRootSingnature != nullptr)
		return;

	std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
	staticSamplers.push_back(getLinearClampStaticSampler(0));
	dx12lib::RootSignatureDescHelper desc = { staticSamplers };
	desc.resize(3);
	desc[CB_Setting].InitAsConstants(4, 0, 0);
	desc[SR_Input].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	desc[UA_Output].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
	auto pSharedDevice = pDevice.lock();
	_pRootSingnature = pSharedDevice->createRootSignature(desc);
}

void FXAA::tryBuildConsolePSO(std::weak_ptr<dx12lib::Device> pDevice) {
	if (_pConsolePSO != nullptr)
		return;

	tryBuildRootSignature(pDevice);
	auto fxaaCsHlsl = getD3DResource("shader/FXAACS.hlsl");
	auto pSharedDevice = pDevice.lock();
	_pConsolePSO = pSharedDevice->createComputePSO("FXAAConsolePSO");
	_pConsolePSO->setRootSignature(_pRootSingnature);
	_pConsolePSO->setComputeShader(compileShader(
		fxaaCsHlsl.begin(),
		fxaaCsHlsl.size(),
		nullptr,
		"FXAAConsole",
		"cs_5_0"
	));
	_pConsolePSO->finalize();
}

void FXAA::updateFXAASetting(dx12lib::ComputeContextProxy pComputeCtx) const {
	pComputeCtx->setCompute32BitConstants(CB_Setting, 1, &_minThreshold, 0);
	pComputeCtx->setCompute32BitConstants(CB_Setting, 1, &_threshold, 1);
	pComputeCtx->setCompute32BitConstants(CB_Setting, 1, &_consoleTangentScale, 2);
	pComputeCtx->setCompute32BitConstants(CB_Setting, 1, &_gSharpness, 3);
}

}
