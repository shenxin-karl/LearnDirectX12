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
	float  _length;      // ����
	float  _omega;       // ��Ƶ��
	float  _speed;       // ����; �ೣ����ʾ�� 
	float  _amplitude;   // ���
	float3 _direction;   // ����
	float  _steep;       // ���Ͷ�
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

struct BillBoardVertex {
	float3 position;
	float2 size;
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
	d3d::Material	  material;
};

struct RenderItem {
	std::shared_ptr<d3d::Mesh> _pMesh;
	GPUStructCBPtr<CBObjectType> _pConstantBuffer;
	std::shared_ptr<dx12lib::ShaderResourceBuffer> _pAlbedoMap;
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
	void renderWaterPass(dx12lib::CommandListProxy pCmdList);
	void drawOpaqueRenderItems(dx12lib::CommandListProxy pCmdList, 
		const std::string &passName,
		D3D_PRIMITIVE_TOPOLOGY primitiveType
	);
private:
	void buildCamera();
	void buildConstantBuffer(dx12lib::CommandListProxy pCmdList);
	void buildTexturePSO(dx12lib::CommandListProxy pCmdList);
	void buildWaterPSO(dx12lib::CommandListProxy pCmdList);
	void buildClipPSO(dx12lib::CommandListProxy pCmdList);
	void buildTreeBillboardPSO(dx12lib::CommandListProxy pCmdList);
	void buildGeometrys(dx12lib::CommandListProxy pCmdList);
	void buildTreeBillBoards(dx12lib::CommandListProxy pCmdList);
	void loadTextures(dx12lib::CommandListProxy pCmdList);
	void buildMaterials();
	void buildRenderItems(dx12lib::CommandListProxy pCmdList);
private:
	std::unique_ptr<d3d::CoronaCamera> _pCamera;
	std::unique_ptr<d3d::BlurFilter>  _pBlurFilter;
	GPUStructCBPtr<d3d::PassCBType>    _pPassCB;
	GPUStructCBPtr<d3d::LightCBType>   _pLightCB;
	GPUStructCBPtr<WaterCBType>        _pWaterCB;
	std::map<std::string, d3d::Material> _materialMap;
	std::map<std::string, std::shared_ptr<d3d::Mesh>> _geometryMap;
	std::map<std::string, std::vector<RenderItem>> _renderItemMap;
	std::map<std::string, std::shared_ptr<dx12lib::GraphicsPSO>> _psoMap;
	std::map<std::string, std::shared_ptr<dx12lib::ShaderResourceBuffer>> _textureMap;
};