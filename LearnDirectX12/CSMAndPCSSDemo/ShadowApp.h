#pragma once
#include <Dx12lib/dx12libStd.h>
#include <BaseApp/BaseApp.h>
#include <D3D/d3dutil.h>
#include <D3D/Shader/ShaderCommon.h>
#include <RenderGraph/RenderGraphStd.h>
#include "RenderGraph/Technique/TechniqueType.hpp"
#include <RenderGraph/Pass/RenderQueuePass.h>
#include "D3D/Model/MeshModel/MeshModel.h"
#include "RenderGraph/Material/Material.h"
#include "RenderGraph/Pass/ClearPass.hpp"
#include "RenderGraph/Pass/PresentPass.hpp"
#include "RenderGraph/RenderGraph/RenderGraph.h"

namespace d3d {
class CSMShadowPass;
}

class ShadowApp : public com::BaseApp {
public:
	ShadowApp();
	~ShadowApp() override;
	auto &getDevice() const { return _pDevice; }
	auto &getSwapChain() const { return _pSwapChain; }
	auto &getRenderGraph() const { return _pRenderGraph; }
	auto &getEnvMap() const { return _pEnvMap; }
	auto &getCamera() const { return _pCamera; }
	auto &getPassCb() const { return _pPassCb; }
	auto &getLightCb() const { return _pLightCb; }
private:
	void onInitialize(dx12lib::DirectContextProxy pDirectCtx) override;
	void onDestroy() override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) override;
private:
	bool _bMouseLeftPress = false;
	std::shared_ptr<d3d::FirstPersonCamera> _pCamera;
	std::shared_ptr<dx12lib::ConstantBuffer> _pLightCb;
	std::shared_ptr<dx12lib::IDepthStencil2DArray> _pShadowMapArray;
	std::shared_ptr<dx12lib::ITextureResourceCube> _pEnvMap;
	dx12lib::FRConstantBufferPtr<d3d::CBPassType> _pPassCb;
	d3d::CSMShadowPass *_pCSMShadowPass;
	Math::BoundingBox _lightBoundingBox;
	std::shared_ptr<d3d::MeshModel> _pMeshModel;
	std::shared_ptr<rgph::RenderGraph> _pRenderGraph;
};
