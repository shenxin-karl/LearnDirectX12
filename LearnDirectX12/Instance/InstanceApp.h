#pragma once
#include "dx12lib/dx12libStd.h"
#include "BaseApp/BaseApp.h"
#include "D3D/Camera.h"
#include "D3D/Mesh.h"
#include "D3D/ShaderCommon.h"
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
	std::shared_ptr<d3d::Mesh> pMesh;
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
	void buildBuffer(dx12lib::CommandContextProxy pCommonCtx);
	void loadTextures(dx12lib::CommandContextProxy pCommonCtx);
	void loadSkull(dx12lib::GraphicsContextProxy pGraphicsCtx);
	void buildPSO();
	void buildRenderItem();
private:
	constexpr static size_t kMaxInstanceSize = 150;

	std::unique_ptr<d3d::FirstPersonCamera> _pCamera;
	std::shared_ptr<dx12lib::GraphicsPSO>   _pInstancePSO;

	std::unordered_map<std::string, size_t> _textureIndexMap;
	std::vector<std::shared_ptr<dx12lib::ShaderResourceBuffer>> _textures;

	std::unordered_map<std::string, size_t> _materialIndexMap;
	std::vector<d3d::Material> _materials;

	std::unordered_map<std::string, std::shared_ptr<d3d::Mesh>> _geometryMap;

	std::shared_ptr<dx12lib::ConstantBuffer>   _pLightCB;
	std::shared_ptr<dx12lib::StructuredBuffer> _pMaterialData;
	FRConstantBufferPtr<d3d::PassCBType>       _pPassCB;
	FRStructuredBufferPtr<InstanceData>        _pInstanceBuffer;
	std::vector<RenderItem> _opaqueRenderItems;

	bool _bMouseLeftPress = false;
};