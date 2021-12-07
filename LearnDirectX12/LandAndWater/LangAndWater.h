#include "BaseApp/BaseApp.h"
#include "D3D/d3dulti.h"
#include "Math/VectorHelper.h"
#include "Math/MatrixHelper.h"
#include "Math/MathHelper.h"
#include "D3D/RenderItem.h"
#include "D3D/FrameResource.h"
#include <d3d12.h>

using namespace vec;
using namespace mat;

struct LandVertex {
	float3	position;
	float4	color;
};

struct WaterVertex {
	float3	position;
	float3	normal;
};

struct Shader {
	WRL::ComPtr<ID3DBlob> pVsByteCode;
	WRL::ComPtr<ID3DBlob> pPsByteCode;
public:
	D3D12_SHADER_BYTECODE getVsByteCode() const;
	D3D12_SHADER_BYTECODE getPsByteCode() const;
};

namespace com {
class GameTimer;
}

class LangAndWater : public com::BaseApp {
public:
	virtual bool initialize() override;
	virtual void beginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void tick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void endTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void onResize(int width, int height) override;
private:
	void buildFrameResource();
	void buildLandGeometry();
	void buildWaterGeometry();
	void buildRenderItems();
	void buildShaderAndInputLayout();
	void buildRootSignature();
	void buildPSO();
	void updatePassConstantBuffer(std::shared_ptr<com::GameTimer> pGameTimer);
	void updateObjectConstantBuffer();
	void updateViewMatrix();
	static float getHillsHeight(float x, float z);
	void handleEvent();
	void onMouseMove(POINT point);
	void onMouseLPress();
	void onMouseLRelease();
	void onCharacter(char character);
public:
	std::unordered_map<std::string, WRL::ComPtr<ID3D12PipelineState>> PSOs_;
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> geometrices_;
	std::unordered_map<std::string, Shader> shaders_;

	WRL::ComPtr<ID3D12RootSignature> pRootSignature_;

	PassConstants mainPassCB_;
	std::vector<FrameResource> frameResources_;
	UINT currentFrameResourceIndex_ = 0;
	FrameResource *currentFrameResource_ = nullptr;

	std::vector<std::unique_ptr<d3dUlti::RenderItem>> allRenderItem_;
	std::vector<d3dUlti::RenderItem *> opaqueItems_;
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout_;
	std::vector<D3D12_INPUT_ELEMENT_DESC> waterInputLayout_;

	float3 eyePos_;
	float4x4 view_ = MathHelper::identity4x4();
	float4x4 proj_ = MathHelper::identity4x4();

	bool isWireDraw_ = false;

	float zNear_ = 0.1f;
	float zFar_ = 100.f;

	float radius_ = 15.f;
	float phi_ = 0.f;
	float theta_ = 0.f;
	bool isLeftPressed_ = false;
	POINT lastMousePos_;

};