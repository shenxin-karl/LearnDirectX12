#include "BaseApp/BaseApp.h"
#include "D3D/d3dutil.h"
#include "Math/VectorHelper.h"
#include "Math/MatrixHelper.h"
#include "Math/MathHelper.h"
#include "D3D/RenderItem.h"
#include "D3D/FrameResource.h"
#include <d3d12.h>
#include <bitset>

using namespace vec;
using namespace mat;

struct LandVertex {
	float3	position;
	float4	color;
	float3  normal;
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

class LandAndWater : public com::BaseApp {
public:
	virtual bool initialize() override;
	virtual void beginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void tick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void onResize(int width, int height) override;
	virtual ~LandAndWater() override;
	void waitFrameResource();
private:
	void buildFrameResource();
	void buildLandGeometry();
	void buildWaterGeometry();
	void buildMaterial();
	void buildRenderItems();
	void buildShaderAndInputLayout();
	void buildRootSignature();
	void buildPSO();

	void updateLight(std::shared_ptr<com::GameTimer> pGameTimer);
	void updatePassConstantBuffer(std::shared_ptr<com::GameTimer> pGameTimer);
	void updateObjectConstantBuffer();
	void updateMaterialConstantBuffer();
	void updateViewMatrix();
	void drawLand();
	void drawWater();
	static float getHillsHeight(float x, float z);
	static float3 getHillsNormal(float x, float z);
	void handleEvent();
	void onMouseMove(POINT point);
	void onMouseLPress();
	void onMouseWheel(float offset);
	void onMouseLRelease();
	void onCharacter(char character);
	void onKeyDown(unsigned char key);
	void onKeyUp(unsigned char key);
public:
	std::unordered_map<std::string, WRL::ComPtr<ID3D12PipelineState>> PSOs_;
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> geometrices_;
	std::unordered_map<std::string, std::unique_ptr<d3dUtil::Material>> materials_;
	std::unordered_map<std::string, Shader> shaders_;

	WRL::ComPtr<ID3D12RootSignature> pRootSignature_;

	d3dUtil::PassConstants mainPassCB_;
	std::vector<std::unique_ptr<d3dUtil::FrameResource>> frameResources_;
	UINT currentFrameResourceIndex_ = 0;
	d3dUtil::FrameResource *currentFrameResource_ = nullptr;

	std::vector<std::unique_ptr<d3dUtil::RenderItem>> allRenderItem_;
	std::vector<d3dUtil::RenderItem *> opaqueItems_;
	std::vector<d3dUtil::RenderItem *> waterItems_;
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout_;
	std::vector<D3D12_INPUT_ELEMENT_DESC> waterInputLayout_;
	std::vector<D3D_SHADER_MACRO>		  lightMacro_;

	float3 eyePos_;
	float4x4 view_ = MathHelper::identity4x4();
	float4x4 proj_ = MathHelper::identity4x4();

	bool isLandWireDraw_ = false;
	bool isWaterWireDraw_ = false;

	float zNear_ = 0.1f;
	float zFar_ = 300.f;

	float radius_ = 100.f;
	float phi_ = 0.f;
	float theta_ = 40.f;
	
	float lightPhi_ = 0.0f;
	float lightTheta_ = 40.f;
	bool lightDirty_ = true;
	bool isLeftPressed_ = false;
	POINT lastMousePos_;

	constexpr static int kKeyBitFlag = sizeof(unsigned char) * 256;
	std::bitset<kKeyBitFlag> keyFlag_;
};