#pragma once
#include <Dx12lib/dx12libStd.h>
#include <BaseApp/BaseApp.h>
#include <D3D/d3dutil.h>
#include <D3D/Shader/ShaderCommon.h>
#include <RenderGraph/RenderGraphStd.h>
#include "RenderGraph/Technique/TechniqueType.hpp"
#include <RenderGraph/Pass/RenderQueuePass.h>
#include "D3D/Model/Material/Material.h"
#include "D3D/Model/MeshModel/MeshModel.h"
#include "RenderGraph/Pass/ClearPass.hpp"
#include "RenderGraph/Pass/PresentPass.hpp"

using namespace Math;

struct OpaquePass : public rgph::RenderQueuePass {
	OpaquePass(const std::string &passName)
	: RenderQueuePass(passName)
	, pShadowMap(this, "ShadowMap")
	{
	}
	void link(dx12lib::ICommonContext &commonCtx) const override;
	void reset() override;
public:
	rgph::PassResourcePtr<dx12lib::IShaderResource2D> pShadowMap;
};



struct ShadowMaterial : public d3d::Material {
	ShadowMaterial(const std::string &name, d3d::INode *pNode, d3d::RenderItem *pRenderItem);
	static void initializePso(dx12lib::DirectContextProxy pDirectCtx);
	static void destroyPso();
private:
	static inline std::shared_ptr<dx12lib::GraphicsPSO> pOpaquePSO;
	static inline std::shared_ptr<dx12lib::GraphicsPSO> pShadowPSO;
};



class ShadowApp : public com::BaseApp {
public:
	ShadowApp();
	~ShadowApp() override;
private:
	void onInitialize(dx12lib::DirectContextProxy pDirectCtx) override;
	void onDestroy() override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) override;
private:
	void loadModel(dx12lib::DirectContextProxy pDirectCtx);
	void buildPass();
private:
	bool _bMouseLeftPress = false;
	std::shared_ptr<d3d::FirstPersonCamera> _pCamera;
	std::shared_ptr<dx12lib::DepthStencil2D> _pShadowMap;
	std::shared_ptr<dx12lib::ConstantBuffer> _pLightCb;
	dx12lib::FRConstantBufferPtr<d3d::CBPassType> _pPassCb;
	std::unordered_map<std::string, std::shared_ptr<dx12lib::GraphicsPSO>> _psoMap;

	std::vector<std::shared_ptr<rgph::Pass>> _passes;
	std::shared_ptr<d3d::MeshModel> _pMeshModel;
};
