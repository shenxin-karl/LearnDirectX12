#pragma once
#include "BaseApp/BaseApp.h"
#include "D3D/d3dutil.h"
#include "dx12lib/dx12libStd.h"
#include "dx12lib/StructuredConstantBuffer.hpp"

class BasicTessellationApp : public com::BaseApp {
public:
	BasicTessellationApp();
	~BasicTessellationApp() override;
protected:
	void onInitialize(dx12lib::DirectContextProxy pCmdList) override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onResize(dx12lib::DirectContextProxy pCmdList, int width, int height) override;
private:
	std::unique_ptr<d3d::CoronaCamera>    _pCamera;
	GPUStructCBPtr<d3d::PassCBType>		  _pPassCB;
	std::shared_ptr<dx12lib::GraphicsPSO> _pTessellationPSO;
	std::shared_ptr<d3d::Mesh>			  _pQuadMesh;
};