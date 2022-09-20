#include "ShadowMaterial.h"
#include "ShadowApp.h"
#include "Dx12lib/Pipeline/ShaderRegister.hpp"
#include "RenderGraph/Bindable/ConstantBufferBindable.h"
#include "RenderGraph/Bindable/SamplerTextureBindable.h"
#include "RenderGraph/Technique/Technique.h"
#include "ShadowRgph.h"
#include "D3D/Shadow/CSMShadowPass.h"
#include "D3D/TextureManager/TextureManager.h"
#include "Dx12lib/Device/SwapChain.h"
#include "Dx12lib/Pipeline/PipelineStateObject.h"
#include "Dx12lib/Pipeline/RootSignature.h"
#include "RenderGraph/Pass/SubPass.h"
#include "D3D/Shadow/CSMShadowPass.h"
#include "Dx12lib/Texture/Texture.h"


ShadowMaterial::ShadowMaterial(dx12lib::IDirectContext &directCtx, std::shared_ptr<dx12lib::Texture> pDiffuseTex)
	: rgph::Material("ShadowMaterial")
{
	_vertexInputSlots = pOpaqueSubPass->getVertexDataInputSlots() | pShadowSubPass->getVertexDataInputSlots();
	_pCbObject = directCtx.createFRConstantBuffer<CbObject>(CbObject{});
	auto pAlbedoBindable = rgph::SamplerTextureBindable::make2D(
		dx12lib::RegisterSlot::SRV0, pDiffuseTex
	);

	auto pCbObjectBindable = rgph::ConstantBufferBindable::make(
		dx12lib::RegisterSlot::CBV1, _pCbObject
	);

	// opaque 
	auto pOpaqueTechnique = std::make_shared<rgph::Technique>("Opaque", ShadowRgph::kOpaque);
	{
		auto pStep = std::make_unique<rgph::Step>(pOpaqueSubPass.get());
		pStep->addBindable(pAlbedoBindable);
		pStep->addBindable(pCbObjectBindable);
		pOpaqueTechnique->addStep(std::move(pStep));
	}
	_techniques.push_back(pOpaqueTechnique);

	// shadow
	auto pShadowTechnique = std::make_shared<rgph::Technique>("Shadow", ShadowRgph::kShadow);
	{
		auto pStep = std::make_unique<rgph::Step>(pShadowSubPass.get());
		pShadowTechnique->addStep(std::move(pStep));
	}
	_techniques.push_back(pShadowTechnique);
}

void ShadowMaterial::init(ShadowApp *pApp) {
	auto pSharedDevice = pApp->getDevice();
	auto pSwapChain = pApp->getSwapChain();
	const auto pRenderGraph = pApp->getRenderGraph();

	auto pCSMShadowPass = dynamic_cast<d3d::CSMShadowPass *>(
		pRenderGraph->getRenderQueuePass(ShadowRgph::ShadowPass)
	);

	/// ShadowMaterial::pOpaquePso
	{
		auto pRootSignature = pSharedDevice->createRootSignature(2, 8);
		pRootSignature->at(0).initAsDescriptorTable({
			{ dx12lib::RegisterSlot::CBV0, 1 }, // cbTransform
			{ dx12lib::RegisterSlot::CBV1, 1 }, // cbObject
			{ dx12lib::RegisterSlot::SRV0, 1 }, // gAlbedoMap
		});
		pRootSignature->at(1).initAsDescriptorTable({
			{ dx12lib::RegisterSlot::CBV2, 1 }, // cbPass
			{ dx12lib::RegisterSlot::CBV3, 1 }, // cbLight
			{ dx12lib::RegisterSlot::CBV4, 1 }, // cbShadow
			{ dx12lib::RegisterSlot::SRV1, 1 }, // gShadowMapArray
		});
		pRootSignature->initStaticSampler(0, d3d::getStaticSamplers());
		pRootSignature->finalize();

		auto pOpaquePso = pSharedDevice->createGraphicsPSO("OpaquePso");
		pOpaquePso->setRenderTargetFormat(
			pSwapChain->getRenderTargetFormat(), 
			pSwapChain->getDepthStencilFormat()
		);
		pOpaquePso->setRootSignature(pRootSignature);
		pOpaquePso->setInputLayout({
			d3d::PositionSemantic,
			d3d::NormalSemantic,
			d3d::Texcoord0Semantic,
		});
		pOpaquePso->setVertexShader(d3d::compileShader(
			L"shaders/BlinnPhong.hlsl",
			nullptr,
			"VS",
			"vs_5_0")
		);
		pOpaquePso->setPixelShader(d3d::compileShader(
			L"shaders/BlinnPhong.hlsl",
			nullptr,
			"PS",
			"ps_5_0"
		));
		pOpaquePso->finalize();
		ShadowMaterial::pOpaquePso = pOpaquePso;
	}
	/// ShadowMaterial::pShadowPso
	{

		auto pRootSignature = pSharedDevice->createRootSignature(2);
		pRootSignature->at(0).initAsDescriptorTable({
			{ dx12lib::RegisterSlot::CBV0, 1 }, // cbTransform
		});
		pRootSignature->at(1).initAsDescriptorTable({
			{ dx12lib::RegisterSlot::CBV1, 1 }, // cbPass
		});
		pRootSignature->finalize();

		auto pShadowPso = pSharedDevice->createGraphicsPSO("ShadowPSO");
		pShadowPso->setRootSignature(pRootSignature);
		pShadowPso->setDepthTargetFormat(pCSMShadowPass->getShadowMapFormat());
		pShadowPso->setInputLayout({ d3d::PositionSemantic });

		CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
		rasterizerDesc.DepthBias = 100;
		rasterizerDesc.DepthBiasClamp = 0.f;
		rasterizerDesc.SlopeScaledDepthBias = 1.f;

		pShadowPso->setRasterizerState(rasterizerDesc);
		pShadowPso->setVertexShader(d3d::compileShader(
			L"shaders/Shadows.hlsl",
			nullptr,
			"VS",
			"vs_5_0"
		));
		pShadowPso->setPixelShader(d3d::compileShader(
			L"shaders/Shadows.hlsl",
			nullptr,
			"PS",
			"ps_5_0"
		));
		pShadowPso->finalize();
		ShadowMaterial::pShadowPso = pShadowPso;
	}

	auto pLightCbBindable = rgph::ConstantBufferBindable::make(
		dx12lib::RegisterSlot::CBV3,
		pApp->getLightCb()
	);

	/// ShadowMaterial::pOpaqueSubPass
	{
		auto pCbShadowBindable = rgph::ConstantBufferBindable::make(
			dx12lib::RegisterSlot::CBV4, 
			pCSMShadowPass->getShadowTypeCBuffer()
		);

		rgph::VertexInputSlots vertexInputSlots;
		vertexInputSlots.set(d3d::PositionSemantic.slot);
		vertexInputSlots.set(d3d::Texcoord0Semantic.slot);
		vertexInputSlots.set(d3d::NormalSemantic.slot);
		auto pOpaqueSubPass = std::make_shared<rgph::SubPass>(ShadowMaterial::pOpaquePso);
		pOpaqueSubPass->setPassCBufferShaderRegister(dx12lib::RegisterSlot::CBV2);
		pOpaqueSubPass->setTransformCBufferShaderRegister(dx12lib::RegisterSlot::CBV0);
		pOpaqueSubPass->setVertexDataInputSlots(vertexInputSlots);
		pOpaqueSubPass->addBindable(pLightCbBindable);
		pOpaqueSubPass->addBindable(pCbShadowBindable);
		pOpaqueSubPass->setOnBindCallback(&opaqueSubPassOnBindCallback);
		ShadowMaterial::pOpaqueSubPass = pOpaqueSubPass;
		pRenderGraph->getRenderQueuePass("OpaquePass")->addSubPass(pOpaqueSubPass);
	}
	/// ShadowMaterial::pShadowSubPass
	{
		rgph::VertexInputSlots vertexInputSlots;
		vertexInputSlots.set(d3d::PositionSemantic.slot);
		auto pShadowSubPass = std::make_shared<rgph::SubPass>(ShadowMaterial::pShadowPso);
		pShadowSubPass->setPassCBufferShaderRegister(dx12lib::RegisterSlot::CBV1);
		pShadowSubPass->setTransformCBufferShaderRegister(dx12lib::RegisterSlot::CBV0);
		pShadowSubPass->setVertexDataInputSlots(vertexInputSlots);
		ShadowMaterial::pShadowSubPass = pShadowSubPass;
		pRenderGraph->getRenderQueuePass("ShadowPass")->addSubPass(pShadowSubPass);
	}
}

d3d::MeshModel::MaterialCreator ShadowMaterial::getShadowMaterialCreator(dx12lib::DirectContextProxy pDirectCtx) {
	return [&](const d3d::ALMaterial *pAlMaterial) -> std::shared_ptr<rgph::Material> {
		const auto &diffuseMap = pAlMaterial->getDiffuseMap();
		auto pTex = d3d::TextureManager::instance()->get(diffuseMap.path);
		if (pTex == nullptr) {
			if (diffuseMap.pTextureData != nullptr) {
				pTex = pDirectCtx->createTextureFromMemory(diffuseMap.textureExtName,
					diffuseMap.pTextureData.get(),
					diffuseMap.textureDataSize
				);
			}
			else {
				pTex = pDirectCtx->createTextureFromFile(std::to_wstring(diffuseMap.path), true);
			}
			d3d::TextureManager::instance()->set(diffuseMap.path, pTex);
		}

		assert(pTex->getDimension() == dx12lib::TextureDimension::TEXTURE_2D && pTex->getDepthOrArraySize() == 1);
		return std::make_shared<ShadowMaterial>(*pDirectCtx, pTex);
	};
}


void ShadowMaterial::opaqueSubPassOnBindCallback(const rgph::SubPass *pSubPass, 
	dx12lib::IGraphicsContext &graphicsCtx)
{
	auto *pGraphicsPass = pSubPass->getGraphicsPass();
	auto *pPassResource = pGraphicsPass->getPassResource("ShadowMapArray");
	auto pShadowMapArray = pPassResource->cast<dx12lib::IDepthStencil2DArray>();
	graphicsCtx.setShaderResourceView(dx12lib::RegisterSlot::SRV1, pShadowMapArray->getSRV(0));
}

