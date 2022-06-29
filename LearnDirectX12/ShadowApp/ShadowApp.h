#pragma once
#include <dx12lib/dx12libStd.h>
#include <BaseApp/BaseApp.h>
#include <D3D/d3dutil.h>
#include <D3D/Shader/ShaderCommon.h>

using namespace Math;

class ShadowApp : public com::BaseApp {
public:
	ShadowApp();
	~ShadowApp() override = default;
private:
	void onInitialize(dx12lib::DirectContextProxy pDirectCtx) override;
	void onDestroy() override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) override;
private:
	void loadModel(dx12lib::DirectContextProxy pDirectCtx);
private:
	std::shared_ptr<d3d::FirstPersonCamera> _pCamera;
	std::shared_ptr<dx12lib::DepthStencil2D> _pShadowMap;
	std::shared_ptr<dx12lib::ConstantBuffer> _pLightCb;
	dx12lib::FRConstantBufferPtr<d3d::CBPassType> _pPassCb;
	std::unordered_map<std::string, std::shared_ptr<d3d::Mesh>> _meshMap;
	std::unordered_map<std::string, std::shared_ptr<dx12lib::IShaderResource>> _textureMap;
};
