#pragma once
#include <dx12lib/dx12libStd.h>
#include <BaseApp/BaseApp.h>
#include <D3D/d3dutil.h>
#include <D3D/Shader/ShaderCommon.h>
#include <D3D/Model/ModelInterface.hpp>
#include <RenderGraph/RenderGraphStd.h>

#include "RenderGraph/Technique/TechniqueType.hpp"

namespace d3d {
class StaticModel;
}

using namespace Math;

struct CbObjectType {
	float4x4 matWorld = float4x4::identity();
	float4x4 matNormal = float4x4::identity();;
	float4x4 matTexCoord = float4x4::identity();;
	d3d::MaterialData materialData = d3d::MaterialData::defaultMaterialData;
};

class Node {
public:
	explicit Node(dx12lib::IGraphicsContext &graphicsCtx, std::shared_ptr<d3d::StaticModel> pStaticModel);
	void buildOpaqueTechnique(std::shared_ptr<rg::SubPass> pSubPass) const;
	void buildShadowTechnique(std::shared_ptr<rg::SubPass> pSubPass) const;
	void submit(const rg::TechniqueFlag &techniqueFlag) const;
private:
	std::shared_ptr<d3d::StaticModel> _pStaticModel;
	dx12lib::FRConstantBufferPtr<CbObjectType> _pCbObject;
	std::vector<std::unique_ptr<rg::Drawable>> _drawables;
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
	void buildPSOAndSubPass();
	void buildNodes(dx12lib::DirectContextProxy pDirectCtx);
private:
	bool _bMouseLeftPress = false;
	std::unique_ptr<d3d::D3DInitializer> _pd3dInitializer;
	std::shared_ptr<d3d::FirstPersonCamera> _pCamera;
	std::shared_ptr<dx12lib::DepthStencil2D> _pShadowMap;
	std::shared_ptr<dx12lib::ConstantBuffer> _pLightCb;
	dx12lib::FRConstantBufferPtr<d3d::CBPassType> _pPassCb;
	std::unordered_map<std::string, std::shared_ptr<d3d::IModel>> _modelMap;

	std::vector<std::unique_ptr<Node>> _nodes;
	std::shared_ptr<rg::RenderQueuePass> _pShadowPass;
	std::shared_ptr<rg::RenderQueuePass> _pOpaquePass;
};
