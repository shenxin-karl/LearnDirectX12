#include "SkyBoxPass.h"

#include "SkyBox.h"
#include "D3D/Shader/D3DShaderResource.h"
#include "D3D/Shader/ShaderCommon.h"
#include "D3D/Tool/Camera.h"
#include "Dx12lib/Pipeline/PipelineStateObject.h"
#include "Dx12lib/Pipeline/RootSignature.h"
#include "Dx12lib/Pipeline/ShaderRegister.hpp"

namespace d3d {

SkyBoxPass::SkyBoxPass(const std::string &passName) : GraphicsPass(passName, true, true)  {
}

void SkyBoxPass::execute(dx12lib::DirectContextProxy pDirectCtx) {
	assert(pEnvMap != nullptr);
	assert(pCamera != nullptr);
	assert(renderTargetFormat != DXGI_FORMAT_UNKNOWN);
	assert(depthStencilFormat != DXGI_FORMAT_UNKNOWN);

	GraphicsPass::execute(pDirectCtx);

	if (_pGraphicsPso == nullptr) 
		buildGraphicsPso(pDirectCtx);
	if (_pCubeVertexBuffer == nullptr)
		_pCubeVertexBuffer = SkyBox::buildCubeVertexBuffer(pDirectCtx);

	pDirectCtx->setGraphicsPSO(_pGraphicsPso);
	pDirectCtx->setVertexBuffer(_pCubeVertexBuffer);
	pDirectCtx->setShaderResourceView(dx12lib::RegisterSlot::SRV0, pEnvMap->getSRV());
	pDirectCtx->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto matView = pCamera->getView();
	matView._41 = 0.f; matView._42 = 0.f; matView._43 = 0.f;
	Matrix4 view(matView);
	Matrix4 proj(pCamera->getProj());
	float4x4 viewProj = static_cast<float4x4>(proj * view);

	pDirectCtx->setGraphics32BitConstants(dx12lib::RegisterSlot::CBV0, 
		sizeof(float4x4) / sizeof(float),
		&viewProj
	);

	pDirectCtx->drawInstanced(
		_pCubeVertexBuffer->getVertexCount(),
		1,
		0
	);
}

void SkyBoxPass::buildGraphicsPso(dx12lib::DirectContextProxy pDirectCtx) {
	auto pSharedDevice = pDirectCtx->getDevice().lock();
	auto pRootSignature = pSharedDevice->createRootSignature(2, 1);
	pRootSignature->initStaticSampler(0, d3d::getLinearWrapStaticSampler(0));
	pRootSignature->at(0).initAsConstants(
		dx12lib::RegisterSlot::CBV0,
		sizeof(float4x4) / sizeof(float)
	);
	pRootSignature->at(1).initAsDescriptorTable({
		{ dx12lib::RegisterSlot::SRV0, 1 }
	});
	pRootSignature->finalize();

	std::vector<D3D_SHADER_MACRO> macros;
	if (enableToneMapping)
		macros.emplace_back("ENABLE_TONE_MAPPING", nullptr);
	if (enableGammaCorrection)
		macros.emplace_back("ENABLE_GAMMA_CORRECTION", nullptr);
	macros.emplace_back(nullptr, nullptr);
	_pGraphicsPso = pSharedDevice->createGraphicsPSO("SkyBoxPassPSO");
	_pGraphicsPso->setRootSignature(pRootSignature);
	_pGraphicsPso->setRenderTargetFormat(renderTargetFormat, depthStencilFormat);

	CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	_pGraphicsPso->setRasterizerState(rasterizerDesc);

	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	_pGraphicsPso->setDepthStencilState(depthStencilDesc);

	_pGraphicsPso->setInputLayout({
		{ "POSITION",
		0,
		DXGI_FORMAT_R32G32B32_FLOAT,
		0,
		0,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		0
		}
	});

	auto skyBoxContent = getD3DResource("HlslShader/SkyBox.hlsl");
	_pGraphicsPso->setVertexShader(compileShaderParseInclude(
		"HlslShader/SkyBox.hlsl",
		skyBoxContent.begin(),
		skyBoxContent.size(),
		macros.data(),
		"VS",
		"vs_5_0"
	));

	_pGraphicsPso->setPixelShader(compileShaderParseInclude(
		"HlslShader/SkyBox.hlsl",
		skyBoxContent.begin(),
		skyBoxContent.size(),
		macros.data(),
		"PS",
		"ps_5_0"
	));
	_pGraphicsPso->finalize();
}

}
