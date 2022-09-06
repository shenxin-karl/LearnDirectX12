#include <iostream>
#include <map>
#include "BaseApp/BaseApp.h"
#include "dx12lib/dx12libStd.h"
#include "Math/MathHelper.h"
#include "D3D/Shader/ShaderCommon.h"
#include "D3D/d3dutil.h"
#include "D3D/Model/Mesh/Mesh.h"


namespace com {
	struct Vertex;
}

struct WaterParameDesc {
	float        length;
	float        speed;
	float        amplitude;
	Math::float3 direction;
	float        steep;
};

class WaterParame {
	float        _length;      // 波长
	float        _omega;       // 角频率
	float        _speed;       // 波速; 相常数表示法 
	float        _amplitude;   // 振幅
	Math::float3 _direction;   // 方向
	float        _steep;       // 陡峭度
public:
	void init(const WaterParameDesc &desc);
};

constexpr std::size_t kMaxWaterParameCount = 4;
struct WaterCBType {
	WaterParame waterParames[kMaxWaterParameCount];
};

struct MeshVertex {
	Math::float3 position;
	Math::float3 normal;
	Math::float2 texcoord;
public:
	MeshVertex(const com::Vertex &vert);
};

struct BillBoardVertex {
	Math::float3 position;
	Math::float2 size;
};

struct WaterVertex {
	Math::float3 position;
	Math::float3 normal;
public:
	WaterVertex(const com::Vertex &vert);
};

struct CBObjectType {
	Math::float4x4          world;
	Math::float4x4          normalMat;
	Math::float4x4          matTransform;
	d3d::MaterialData	    material;
};

struct RenderItem {
	std::shared_ptr<d3d::Mesh> _pMesh;
	dx12lib::FRConstantBufferPtr<CBObjectType> _pConstantBuffer;
	std::shared_ptr<dx12lib::ITextureResource> _pAlbedoMap;
};

class LandAndWater : public com::BaseApp {
public:
	LandAndWater();
	~LandAndWater();
public:
	void onInitialize(dx12lib::DirectContextProxy pDirectCtx) override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) override;
private:
	void pollEvent();
	void updateConstantBuffer(std::shared_ptr<com::GameTimer> pGameTimer);
	void renderWaterPass(dx12lib::DirectContextProxy pDirectCtx);
	void drawOpaqueRenderItems(dx12lib::DirectContextProxy pDirectCtx,
		const std::string &passName,
		D3D_PRIMITIVE_TOPOLOGY primitiveType
	);
private:
	void buildCamera();
	void buildConstantBuffer(dx12lib::DirectContextProxy pDirectCtx);
	void buildTexturePSO(dx12lib::DirectContextProxy pDirectCtx);
	void buildWaterPSO(dx12lib::DirectContextProxy pDirectCtx);
	void buildClipPSO(dx12lib::DirectContextProxy pDirectCtx);
	void buildTreeBillboardPSO(dx12lib::DirectContextProxy pDirectCtx);
	void buildGeometrys(dx12lib::DirectContextProxy pDirectCtx);
	void buildTreeBillBoards(dx12lib::DirectContextProxy pDirectCtx);
	void loadTextures(dx12lib::DirectContextProxy pDirectCtx);
	void buildMaterials();
	void buildRenderItems(dx12lib::DirectContextProxy pDirectCtx);
private:
	std::unique_ptr<d3d::BlurFilter> _pBlurFilter;
	FRConstantBufferPtr<WaterCBType> _pWaterCB;
	FRConstantBufferPtr<d3d::CBPassType> _pPassCB;
	FRConstantBufferPtr<d3d::CBLightType> _pLightCB;
	std::unique_ptr<d3d::FirstPersonCamera> _pCamera;
	std::map<std::string, d3d::MaterialData> _materialMap;
	std::map<std::string, std::shared_ptr<d3d::Mesh>> _geometryMap;
	std::map<std::string, std::vector<RenderItem>> _renderItemMap;
	std::map<std::string, std::shared_ptr<dx12lib::GraphicsPSO>> _psoMap;
	std::map<std::string, std::shared_ptr<dx12lib::ITextureResource>> _textureMap;
};