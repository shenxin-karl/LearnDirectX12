#include <iostream>
#include <map>
#include "BaseApp/BaseApp.h"
#include "dx12lib/dx12libStd.h"
#include "dx12lib/StructConstantBuffer.hpp"
#include "Math/MathHelper.h"
#include "D3D/ShaderCommon.h"
#include "D3D/d3dutil.h"
#include "D3D/Mesh.h"

using namespace Math;
namespace com {
struct Vertex;
}

struct WaterParameDesc {
	float length;
	float speed;
	float amplitude;
	float3 direction;
	float steep;
};

class WaterParame {
	float  _length;      // 波长
	float  _omega;       // 角频率
	float  _speed;       // 波速; 相常数表示法 
	float  _amplitude;   // 振幅
	float3 _direction;   // 方向
	float  _steep;       // 陡峭度
public:
	void init(const WaterParameDesc &desc);
};

enum RootParameType {
	CBPass   = 0,
	CBLight  = 1,
	CBObject = 2,
	CBWater  = 3,
	SRAlbedo = 3,
};

constexpr std::size_t kMaxWaterParameCount = 4;
struct WaterCBType {
	WaterParame waterParames[kMaxWaterParameCount];
};

struct MeshVertex {
	float3 position;
	float3 normal;
	float2 texcoord;
public:
	MeshVertex(const com::Vertex &vert);
};

struct WaterVertex {
	float3 position;
	float3 normal;
public:
	WaterVertex(const com::Vertex &vert);
};

struct CBObjectType {
	float4x4          world;
	float4x4          normalMat;
	float4x4          matTransfrom;
	d3dutil::Material material;
};

struct RenderItem {
	std::shared_ptr<d3dutil::Mesh>	  _pMesh;
	GPUStructCBPtr<CBObjectType>      _pConstantBuffer;
	std::shared_ptr<dx12lib::Texture> _pAlbedoMap;
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
	void renderTexturePass(dx12lib::CommandListProxy pCmdList);
	void renderWaterPass(dx12lib::CommandListProxy pCmdList);
	void renderWireBoxPass(dx12lib::CommandListProxy pCmdList);
private:
	void buildCamera();
	void buildConstantBuffer(dx12lib::CommandListProxy pCmdList);
	void buildTexturePSO(dx12lib::CommandListProxy pCmdList);
	void buildWaterPSO(dx12lib::CommandListProxy pCmdList);
	void buildClipPSO(dx12lib::CommandListProxy pCmdList);
	void buildGeometrys(dx12lib::CommandListProxy pCmdList);
	void loadTextures(dx12lib::CommandListProxy pCmdList);
	void buildMaterials();
	void buildRenderItems(dx12lib::CommandListProxy pCmdList);
private:
	std::unique_ptr<d3dutil::CoronaCamera> _pCamera;
	GPUStructCBPtr<d3dutil::PassCBType>    _pPassCB;
	GPUStructCBPtr<d3dutil::LightCBType>   _pLightCB;
	GPUStructCBPtr<WaterCBType>            _pWaterCB;
	std::map<std::string, d3dutil::Material> _materialMap;
	std::map<std::string, std::shared_ptr<d3dutil::Mesh>> _geometryMap;
	std::map<std::string, std::shared_ptr<dx12lib::Texture>> _textureMap;
	std::map<std::string, std::shared_ptr<dx12lib::GraphicsPSO>> _psoMap;
	std::map<std::string, std::vector<RenderItem>> _renderItemMap;
};