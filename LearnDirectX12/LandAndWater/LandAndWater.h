#include <iostream>
#include <map>
#include "BaseApp/BaseApp.h"
#include "dx12lib/dx12libStd.h"
#include "dx12lib/StructConstantBuffer.hpp"
#include "Math/MathHelper.h"
#include "D3D/ShaderCommon.h"
#include "D3D/d3dutil.h"

using namespace Math;

struct LandVertex {
	float3 position;
	float3 normal;
	float2 texcoord;
};

struct WaterVertex {
	float3 position;
	float3 normal;
};

struct Mesh {
	std::shared_ptr<dx12lib::VertexBuffer> _pVertexBuffer;
	std::shared_ptr<dx12lib::IndexBuffer>  _pIndexBuffer;
};

struct RenderItem {
	std::shared_ptr<Mesh>			  _pMesh;
	GPUStructCBPtr<d3dutil::Material> _pConstantBuffer;
};

class LandAndWater : public com::BaseApp {
public:
	LandAndWater();
	~LandAndWater();
public:
	virtual void onInitialize(dx12lib::CommandListProxy pCmdList) override;
	virtual void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void onResize(dx12lib::CommandListProxy pCmdList, int width, int height) override;
private:
	void pollEvent();
	void updateConstantBuffer(std::shared_ptr<com::GameTimer> pGameTimer);
	void renderLandPass(dx12lib::CommandListProxy pCmdList);
	void renderWaterPass(dx12lib::CommandListProxy pCmdList);
private:
	void buildCamera();
	void buildConstantBuffer(dx12lib::CommandListProxy pCmdList);
	void buildPSO(dx12lib::CommandListProxy pCmdList);
	void buildGeometrys(dx12lib::CommandListProxy pCmdList);
	void loadTextures(dx12lib::CommandListProxy pCmdList);
	void buildMaterials();
	void buildRenderItems(dx12lib::CommandListProxy pCmdList);
private:
	std::unique_ptr<d3dutil::CoronaCamera> _pCamera;
	GPUStructCBPtr<d3dutil::PassCBType>    _pPassCB;
	GPUStructCBPtr<d3dutil::LightCBType>   _pLightCB;
	std::map<std::string, d3dutil::Material> _materialMap;
	std::map<std::string, std::shared_ptr<Mesh>> _geometryMap;
	std::map<std::string, std::shared_ptr<dx12lib::Texture>> _textureMap;
	std::map<std::string, std::shared_ptr<dx12lib::GraphicsPSO>> _psoMap;
	std::map<std::string, std::vector<RenderItem>> _renderItemMap;
};