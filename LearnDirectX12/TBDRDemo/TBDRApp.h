#pragma once
#include <BaseApp/BaseApp.h>
#include "D3D/Tool/Camera.h"

namespace d3d {
class MeshModel;
}

namespace rgph {
class RenderGraph;
}

class TBDRApp : public com::BaseApp {
public:
	TBDRApp();
	~TBDRApp() override;
	void onInitialize(dx12lib::DirectContextProxy pDirectCtx) override;
	void onDestroy() override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) override;
	auto getSwapChain() const -> std::shared_ptr<dx12lib::SwapChain>;
private:
	bool _bMouseLeftPress = false;
	std::unique_ptr<d3d::CameraBase> _pCamera;
	std::shared_ptr<dx12lib::ConstantBuffer> _pLightCb;
	dx12lib::FRConstantBufferPtr<d3d::CBPassType> _pPassCb;
	std::shared_ptr<d3d::MeshModel> _pMeshModel;
	std::shared_ptr<rgph::RenderGraph> _pRenderGraph;
public:
	std::shared_ptr<dx12lib::RenderTarget2D> pGBuffer0;
	std::shared_ptr<dx12lib::RenderTarget2D> pGBuffer1;
	std::shared_ptr<dx12lib::RenderTarget2D> pGBuffer2;
	std::shared_ptr<dx12lib::UnorderedAccess2D> pLightingBuffer;
};
