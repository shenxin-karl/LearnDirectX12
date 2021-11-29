#pragma once
#include "BaseApp/BaseApp.h"
#include "GameTimer/GameTimer.h"
#include "Geometry/GeometryGenerator.h"
#include "D3D/d3dulti.h"
#include "D3D/RenderItem.h"
#include "D3D/UploadBuffer.h"
#include "D3D/FrameResource.h"
#include <unordered_map>
#include <wrl.h>

namespace DX = DirectX;
namespace WRL = Microsoft::WRL;

using namespace vec;

struct ShapeVertex {
	float3	position;
	float4	color;
};

class Shape : public com::BaseApp {
public:
	virtual bool initialize();
	virtual void beginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void tick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void endTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void onResize(int width, int height) override;
	virtual ~Shape() override;
private:
	void buildShapeGeometry();
	void buildRenderItems();
private:
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> geometrice_;
	std::unordered_map<std::string, WRL::ComPtr<ID3DBlob>> shaders_;
	std::unordered_map<std::string, WRL::ComPtr<ID3D12PipelineState>> PSOs_;
	std::vector<std::unique_ptr<d3dUlti::RenderItem>> allRenderItems_;
	std::vector<std::unique_ptr<d3dUlti::RenderItem>> opaqueRItems_;
	std::unique_ptr<UploadBuffer<PassConstants>> passCB_;
	std::unique_ptr<UploadBuffer<ObjectConstants>> objCB_;
};