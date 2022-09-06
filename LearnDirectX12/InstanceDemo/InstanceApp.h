#pragma once
#include "dx12lib/dx12libStd.h"
#include "BaseApp/BaseApp.h"
#include "D3D/Shader/ShaderCommon.h"
#include "D3D/Model/Mesh/Mesh.h"
#include "D3D/Tool/Camera.h"
#include "dx12lib/Pipeline/ShaderRegister.hpp"
#include <DirectXCollision.h>


struct OpaqueVertex {
	Math::float3 position;
	Math::float3 normal;
public:
	OpaqueVertex(const com::Vertex &vert) : position(vert.position), normal(vert.normal) {}
};

struct RenderItem {
	Math::float4x4 matWorld;
	DirectX::BoundingBox bounds;
	size_t diffuseMapIdx;
	size_t materialIdx;
	Math::float3 axis;
};

struct InstanceData {
	Math::float4x4 matWorld;
	Math::float4x4 matNormal;
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
	void buildBuffer(dx12lib::CommonContextProxy pCommonCtx);
	void loadTextures(dx12lib::CommonContextProxy pCommonCtx);
	void loadSkull(dx12lib::GraphicsContextProxy pGraphicsCtx);
	void buildMaterial(dx12lib::CommonContextProxy pCommonCtx);
	void buildPSO();
	void buildRenderItem();
	std::vector<RenderItem> cullingByFrustum(std::shared_ptr<com::GameTimer> pGameTimer) const;
	void doDrawInstance(dx12lib::GraphicsContextProxy pGraphicsCtx, 
		std::shared_ptr<d3d::Mesh> pMesh, 
		const std::vector<RenderItem> &renderItems,
		std::shared_ptr<com::GameTimer> pGameTimer
	);
private:
	constexpr static inline size_t kMaxInstanceSize = 150;
	constexpr static inline size_t kMaxTextureArraySize = 5;
	static inline dx12lib::ShaderRegister sInstanceShaderRegister{ dx12lib::RegisterSlot::SRV0, dx12lib::RegisterSpace::Space1 };
	static inline dx12lib::ShaderRegister sMaterialShaderRegister{ dx12lib::RegisterSlot::SRV1, dx12lib::RegisterSpace::Space1 };

	std::unique_ptr<d3d::SkyBox> _pSkyBox;
	std::shared_ptr<d3d::FirstPersonCamera> _pCamera;
	std::shared_ptr<dx12lib::GraphicsPSO>   _pInstancePSO;

	std::vector<std::shared_ptr<dx12lib::SamplerTexture2D>> _textures;
	std::vector<d3d::MaterialData> _materials;

	std::unordered_map<std::string, std::shared_ptr<d3d::Mesh>> _geometryMap;

	std::shared_ptr<dx12lib::ConstantBuffer>   _pLightCB;
	std::shared_ptr<dx12lib::SRStructuredBuffer> _pMaterialData;
	FRConstantBufferPtr<d3d::CBPassType>       _pPassCB;
	FRStructuredBufferPtr<InstanceData>        _pInstanceBuffer;
	std::vector<RenderItem> _opaqueRenderItems;
	bool _bMouseLeftPress = false;
};