#pragma once
#include "dx12lib/dx12libStd.h"
#include "BaseApp/BaseApp.h"
#include "D3D/Shader/ShaderCommon.h"
#include "D3D/Tool/Mesh.h"
#include "D3D/Tool/Camera.h"
#include <DirectXCollision.h>

using namespace Math;
using namespace DirectX;

struct OpaqueVertex {
	float3 position;
	float3 normal;
public:
	OpaqueVertex(const com::Vertex &vert) : position(vert.position), normal(vert.normal) {}
};

struct RenderItem {
	float4x4 matWorld;
	BoundingBox bounds;
	size_t diffuseMapIdx;
	size_t materialIdx;
};

struct InstanceData {
	float4x4 matWorld;
	float4x4 matNormal;
	uint32_t materialIdx;
	uint32_t diffuseMapIdx;
	uint32_t pad0 = 0;
	uint32_t pad1 = 0;
};

enum RootParame : size_t {
	CB_Pass = 0,
	CB_Light = 1,
	SR_InstanceData = 2,
	SR_MaterialData = 3,
	SR_DiffuseMapArray = 4,
};

class InstanceApp : public com::BaseApp {
public:
	InstanceApp();
	~InstanceApp() override;
private:
	void onInitialize(dx12lib::DirectContextProxy pDirectCtx) override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) override;
private:
	void pollEvent();
	void buildCamera();
	void buildBuffer(dx12lib::CommonContextProxy pCommonCtx);
	void loadTextures(dx12lib::CommonContextProxy pCommonCtx);
	void loadSkull(dx12lib::GraphicsContextProxy pGraphicsCtx);
	void buildMaterial(dx12lib::CommonContextProxy pCommonCtx);
	void buildPSO();
	void buildRenderItem();
	std::vector<RenderItem> cullingByFrustum() const;
	void doDrawInstance(dx12lib::GraphicsContextProxy pGraphicsCtx, std::shared_ptr<d3d::Mesh> pMesh, const std::vector<RenderItem> &renderItems);
private:
	constexpr static inline size_t kMaxInstanceSize = 150;
	constexpr static inline size_t kMaxTextureArraySize = 5;

	std::unique_ptr<d3d::SkyBox> _pSkyBox;
	std::shared_ptr<d3d::FirstPersonCamera> _pCamera;
	std::shared_ptr<dx12lib::GraphicsPSO>   _pInstancePSO;

	std::vector<std::shared_ptr<dx12lib::SamplerTexture2D>> _textures;
	std::vector<d3d::Material> _materials;

	std::unordered_map<std::string, std::shared_ptr<d3d::Mesh>> _geometryMap;

	std::shared_ptr<dx12lib::ConstantBuffer>   _pLightCB;
	std::shared_ptr<dx12lib::StructuredBuffer> _pMaterialData;
	FRConstantBufferPtr<d3d::CBPassType>       _pPassCB;
	FRStructuredBufferPtr<InstanceData>        _pInstanceBuffer;
	std::vector<RenderItem> _opaqueRenderItems;
	bool _bMouseLeftPress = false;
};