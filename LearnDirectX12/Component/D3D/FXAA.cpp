#include "FXAA.h"
#include "D3D/D3DShaderResource.h"
#include "D3D/d3dutil.h"
#include "dx12lib/Device.h"
#include "dx12lib/RootSignature.h"
#include "dx12lib/PipelineStateObject.h"
#include "dx12lib/UnorderedAccessBuffer.h"

namespace d3d {

FXAA::FXAA(dx12lib::ComputeContextProxy pComputeCtx, 
	std::uint32_t width, 
	std::uint32_t height,
	DXGI_FORMAT format)
: _width(width), _height(height), _format(format)
{
}

void FXAA::_produceImpl(dx12lib::ComputeContextProxy pComputeCtx,
	std::shared_ptr<dx12lib::IShaderSourceResource> pInput)
{
	assert(pInput != nullptr);
	assert(!pInput->isShaderSample());
	tryBuildConsolePSO(pComputeCtx->getDevice());

	pComputeCtx->transitionBarrier(pInput, D3D12_RESOURCE_STATE_GENERIC_READ);
	pComputeCtx->transitionBarrier(_pOutputMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	pComputeCtx->setComputePSO(_pConsolePSO);
	updateFXAASetting();
	pComputeCtx->setShaderResourceBuffer(pInput, SR_Input);
	pComputeCtx->setUnorderedAccessBuffer(_pOutputMap, UA_Output);
	std::size_t numXGroup = static_cast<std::size_t>(std::ceil(float(_width) / float(kFXAAThreadCount)));
	std::size_t numYGroup = static_cast<std::size_t>(std::ceil(float(_height) / float(kFXAAThreadCount)));
	pComputeCtx->dispatch(numXGroup, numYGroup);
}

void FXAA::tryBuildRootSignature(std::weak_ptr<dx12lib::Device> pDevice) {
	if (_pRootSingnature != nullptr)
		return;

	dx12lib::RootSignatureDescHelper desc;
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
	auto fxaaCsHlsl = getD3DShaderResource("shader/FXAACS.hlsl");
	auto pSharedDevice = pDevice.lock();
	auto pConsolePSO = pSharedDevice->createComputePSO("FXAAConsolePSO");
	pConsolePSO->setRootSignature(_pRootSingnature);
	pConsolePSO->setComputeShader(compileShader(
		fxaaCsHlsl.begin(),
		fxaaCsHlsl.size(),
		nullptr,
		"FXAAConsole",
		"cs_5_0"
	));
	pConsolePSO->finalize();
}

void FXAA::updateFXAASetting() {
	
}

}
