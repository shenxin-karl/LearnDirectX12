#include "SkyBox.h"
#include "D3D/Tool/Camera.h"
#include "D3D/Shader/D3DShaderResource.h"
#include "D3D/Shader/ShaderCommon.h"
#include <dx12lib/Pipeline/PipelineStd.h>
#include <dx12lib/Texture/SamplerTexture.h>
#include <dx12lib/Buffer/VertexBuffer.h>

namespace d3d {

SkyBox::SkyBox(const SkyBoxDesc &desc) : _pCubeMap(desc.pCubeMap) {
	auto pGraphicsCtx = desc.pGraphicsCtx;
	auto pSharedDevice = pGraphicsCtx->getDevice().lock();

	_pViewProj = pGraphicsCtx->createFRConstantBuffer<float4x4>();
	if (_pCubeMap == nullptr)
		_pCubeMap = pGraphicsCtx->createDDSTextureCubeFromFile(desc.filename);


	// build Root Signature
	auto pRootSignature = pSharedDevice->createRootSignature(1, 1);
	pRootSignature->at(0).initAsDescriptorTable({
		{ dx12lib::RegisterSlot::CBV0, 1 },
		{ dx12lib::RegisterSlot::SRV0, 1 },
	});
	pRootSignature->initStaticSampler(0, getLinearWrapStaticSampler(0));
	pRootSignature->finalize();

	// build pso
	_pSkyBoxPSO = pSharedDevice->createGraphicsPSO("SkyBoxPSO");
	_pSkyBoxPSO->setRootSignature(pRootSignature);

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		D3D12_INPUT_ELEMENT_DESC {
			.SemanticName = "POSITION",
			.SemanticIndex = 0,
			.Format = DXGI_FORMAT_R32G32B32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = 0,
			.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0,
		}
	};
	_pSkyBoxPSO->setInputLayout(inputLayout);
	_pSkyBoxPSO->setRenderTargetFormat(desc.renderTargetFormat, desc.depthStencilFormat);

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	_pSkyBoxPSO->setDepthStencilState(depthStencilDesc);

	CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	_pSkyBoxPSO->setRasterizerState(rasterizerDesc);

	D3D_SHADER_MACRO macros[3] = {
		desc.enableGammaCorrection	? "ENABLE_GAMMA_CORRECTION"	 : nullptr, nullptr,
		desc.enableToneMapping		? "ENABLE_TONE_MAPPING"		 : nullptr, nullptr,
		nullptr, nullptr,
	};

	auto skyBoxContent = getD3DResource("HlslShader/SkyBox.hlsl");
	_pSkyBoxPSO->setVertexShader(compileShaderParseInclude(
		"HlslShader/SkyBox.hlsl",
		skyBoxContent.begin(),
		skyBoxContent.size(),
		macros,
		"VS",
		"vs_5_0"
	));

	_pSkyBoxPSO->setPixelShader(compileShaderParseInclude(
		"HlslShader/SkyBox.hlsl",
		skyBoxContent.begin(),
		skyBoxContent.size(),
		macros,
		"PS",
		"ps_5_0"
	));
	_pSkyBoxPSO->finalize();

	buildCubeVertexBuffer(pGraphicsCtx);
}

void SkyBox::render(dx12lib::GraphicsContextProxy pGraphicsCtx, std::shared_ptr<CameraBase> pCamera) const {
	auto matView = pCamera->getView();
	matView._41 = 0.f; matView._42 = 0.f; matView._43 = 0.f;
	XMMATRIX view = XMLoadFloat4x4(&matView);
	XMMATRIX proj = XMLoadFloat4x4(&pCamera->getProj());
	XMStoreFloat4x4(_pViewProj->map(), view * proj);

	pGraphicsCtx->setGraphicsPSO(_pSkyBoxPSO);
	pGraphicsCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV0, _pViewProj);
	pGraphicsCtx->setShaderResourceView(dx12lib::RegisterSlot::SRV0, _pCubeMap->getSRV(0));
	pGraphicsCtx->setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pGraphicsCtx->setVertexBuffer(_pCubeVertexBuffer);
	pGraphicsCtx->drawInstanced(_pCubeVertexBuffer->getVertexCount(), 1, 0);
}

void SkyBox::buildCubeVertexBuffer(dx12lib::GraphicsContextProxy pGraphicsCtx) {
	float3 skyboxVertices[] = {
		{ -1.0f,+1.0f,-1.0f }, { -1.0f,-1.0f,-1.0f }, { +1.0f,-1.0f,-1.0f },
		{ +1.0f,-1.0f,-1.0f }, { +1.0f,+1.0f,-1.0f }, { -1.0f,+1.0f,-1.0f },
		{ -1.0f,-1.0f,+1.0f }, { -1.0f,-1.0f,-1.0f }, { -1.0f,+1.0f,-1.0f },
		{ -1.0f,+1.0f,-1.0f }, { -1.0f,+1.0f,+1.0f }, { -1.0f,-1.0f,+1.0f },
		{ +1.0f,-1.0f,-1.0f }, { +1.0f,-1.0f,+1.0f }, { +1.0f,+1.0f,+1.0f },
		{ +1.0f,+1.0f,+1.0f }, { +1.0f,+1.0f,-1.0f }, { +1.0f,-1.0f,-1.0f },
		{ -1.0f,-1.0f,+1.0f }, { -1.0f,+1.0f,+1.0f }, { +1.0f,+1.0f,+1.0f },
		{ +1.0f,+1.0f,+1.0f }, { +1.0f,-1.0f,+1.0f }, { -1.0f,-1.0f,+1.0f },
		{ -1.0f,+1.0f,-1.0f }, { +1.0f,+1.0f,-1.0f }, { +1.0f,+1.0f,+1.0f },
		{ +1.0f,+1.0f,+1.0f }, { -1.0f,+1.0f,+1.0f }, { -1.0f,+1.0f,-1.0f },
		{ -1.0f,-1.0f,-1.0f }, { -1.0f,-1.0f,+1.0f }, { +1.0f,-1.0f,-1.0f },
		{ +1.0f,-1.0f,-1.0f }, { -1.0f,-1.0f,+1.0f }, { +1.0f,-1.0f,+1.0f },
	};

	_pCubeVertexBuffer = pGraphicsCtx->createVertexBuffer(
		skyboxVertices, 
		std::size(skyboxVertices), 
		sizeof(float3)
	);
}

}
