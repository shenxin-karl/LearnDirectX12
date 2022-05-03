#include "SkyBox.h"

#include "Camera.h"
#include "D3DShaderResource.h"
#include "ShaderCommon.h"
#include "dx12lib/PipelineStateObject.h"
#include "dx12lib/RootSignature.h"
#include "dx12lib/TextureShaderResource.h"
#include "dx12lib/VertexBuffer.h"

namespace d3d {

SkyBox::SkyBox(const SkyBoxDesc &desc) : _pCubeMap(desc.pCubeMap) {
	auto pGraphicsCtx = desc.pGraphicsCtx;
	auto pSharedDevice = pGraphicsCtx->getDevice().lock();

	_pViewProj = pGraphicsCtx->createFRConstantBuffer<float4x4>();
	if (_pCubeMap == nullptr)
		_pCubeMap = pGraphicsCtx->createDDSTextureCubeFromFile(desc.filename);


	// build Root Signature
	std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
	staticSamplers.push_back(getLinearWrapStaticSampler(0));
	dx12lib::RootSignatureDescHelper rootDesc(staticSamplers);
	rootDesc.resize(2);
	rootDesc[CB_Setting].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	rootDesc[SR_CubeMap].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	auto pRootSignature = pSharedDevice->createRootSignature(rootDesc);

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

	auto skyBoxContent = getD3DResource("shader/SkyBox.hlsl");
	_pSkyBoxPSO->setVertexShader(compileShader(
		skyBoxContent.begin(),
		skyBoxContent.size(),
		nullptr,
		"VS",
		"vs_5_0"
	));

	_pSkyBoxPSO->setPixelShader(compileShader(
		skyBoxContent.begin(),
		skyBoxContent.size(),
		nullptr,
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
	pGraphicsCtx->setConstantBufferView(_pViewProj->getConstantBufferView(), CB_Setting);
	pGraphicsCtx->setShaderResourceView(_pCubeMap->getShaderResourceView(), SR_CubeMap);
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
