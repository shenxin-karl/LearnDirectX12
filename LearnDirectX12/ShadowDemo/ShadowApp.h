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

using namespace Math;


namespace TechType {
	constexpr rgph::TechniqueType kOpaque{ 1 };
	constexpr rgph::TechniqueType kShadow{ 2 };
}

struct ShadowPass : rgph::RenderQueuePass {
	explicit ShadowPass(const std::string &passName);
};

struct OpaquePass : rgph::RenderQueuePass {
	OpaquePass(const std::string &passName);
public:
	rgph::PassResourcePtr<dx12lib::IDepthStencil2D> pShadowMap;
};


class ShadowMaterial : public rgph::Material {
public:
	explicit ShadowMaterial(std::shared_ptr<dx12lib::IShaderResource2D> pDiffuseTex);
public:
	static inline std::shared_ptr<dx12lib::GraphicsPSO> pOpaquePso;
	static inline std::shared_ptr<dx12lib::GraphicsPSO> pShadowPso;
	static inline std::shared_ptr<rgph::SubPass> pOpaqueSubPass;
	static inline std::shared_ptr<rgph::SubPass> pShadowSubPass;
	static inline rgph::PassResourceBase *pShadowMap;
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
	void initPso(dx12lib::DirectContextProxy pDirectCtx) const;
	void initSubPass() const;
	void buildPass();
private:
	bool _bMouseLeftPress = false;
	std::shared_ptr<d3d::FirstPersonCamera> _pCamera;
	std::shared_ptr<dx12lib::DepthStencil2D> _pShadowMap;
	std::shared_ptr<dx12lib::ConstantBuffer> _pLightCb;
	dx12lib::FRConstantBufferPtr<d3d::CBPassType> _pPassCb;

	rgph::RenderGraph _graph;
	std::shared_ptr<d3d::MeshModel> _pMeshModel;
};
