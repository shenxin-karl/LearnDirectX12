#pragma once
#include "dx12lib/dx12libStd.h"
#include "D3D/Shader/ShaderCommon.h"
#include "D3D/d3dutil.h"
#include "GameTimer/GameTimer.h"
#include "BaseApp/BaseApp.h"
#include "D3D/Animation/SkinnedData.h"
#include "D3D/Model/Mesh/Mesh.h"


struct ShapeVertex {
	Math::float3 position;
	Math::float3 normal;
	Math::float2 texcoord;
	Math::float3 tangent;
};

struct SkullVertex {
	Math::float3 position;
	Math::float3 normal;
};

struct ObjectCB {
	Math::float4x4        matWorld    = Math::float4x4::identity();
	Math::float4x4		  matNormal   = Math::float4x4::identity();
	Math::float4x4		  matTexCoord = Math::float4x4::identity();
	d3d::MaterialData	  material;
};

struct SkinnedBoneCB {
	constexpr static size_t kMaxCount = 96;
	Math::float4x4 boneTransforms[kMaxCount];
};

struct RenderItem {
	d3d::SubMesh subMesh;
	FRConstantBufferPtr<ObjectCB> pObjectCb;
	std::shared_ptr<dx12lib::VertexBuffer> pVertexBuffer;
	std::shared_ptr<dx12lib::IndexBuffer> pIndexBuffer;
	std::shared_ptr<dx12lib::SamplerTexture2D> pAlbedo;
	std::shared_ptr<dx12lib::SamplerTexture2D> pNormal;
};

class Shape : public com::BaseApp {
public:
	Shape();
	~Shape() override;
	void onInitialize(dx12lib::DirectContextProxy pDirectCtx) override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) override;
private:
	void buildTexturePSO(dx12lib::DirectContextProxy pDirectCtx);
	void buildColorPSO(dx12lib::DirectContextProxy pDirectCtx);
	void buildSkinnedAnimationPSO(dx12lib::DirectContextProxy pDirectCtx);
	void buildRenderItem(dx12lib::DirectContextProxy pDirectCtx);
	void buildGeometry(dx12lib::DirectContextProxy pDirectCtx);
	void buildGameLight(dx12lib::DirectContextProxy pDirectCtx);
	void buildMaterials();
	void buildSkullAnimation();
	void loadModelAndBuildRenderItem(dx12lib::DirectContextProxy pDirectCtx);
	void loadTextures(dx12lib::DirectContextProxy pDirectCtx);
	void renderShapesPass(dx12lib::DirectContextProxy pDirectCtx);
	void renderSkullPass(dx12lib::DirectContextProxy pDirectCtx);
	void renderSkinnedAnimationPass(dx12lib::DirectContextProxy pDirectCtx);
	void pollEvent();
	void updatePassCB(std::shared_ptr<com::GameTimer> pGameTimer);
	void updateSkullAnimationCb(std::shared_ptr<com::GameTimer> pGameTimer);
	void updateSkinnedAnimationCb(std::shared_ptr<com::GameTimer> pGameTimer);
private:
	Math::float4x4 _skullMatWorld;
	d3d::BoneAnimation _skullAnimation;
	float _skullAnimationTimePoint = 0.f;
	FRConstantBufferPtr<ObjectCB> _pSkullObjCB;

	// ½ÇÉ«¶¯»­
	d3d::SkinnedData _skinnedData;
	float _skinnedAnimationTimePoint = 0.f;
	FRConstantBufferPtr<SkinnedBoneCB> _pSkinnedBoneCb;

	std::unique_ptr<d3d::SkyBox>	  _pSkyBox;
	std::unique_ptr<d3d::SobelFilter> _pSobelFilter;
	std::shared_ptr<d3d::CoronaCamera>  _pCamera;
	FRConstantBufferPtr<d3d::CBLightType> _pGameLightsCB;
	FRConstantBufferPtr<d3d::CBPassType>  _pPassCB;
	std::unordered_map<std::string, d3d::MaterialData> _materials;
	std::unordered_map<std::string, std::shared_ptr<d3d::Mesh>> _geometrys;
	std::unordered_map<std::string, std::vector<RenderItem>> _renderItems;
	std::unordered_map<std::string, std::shared_ptr<dx12lib::GraphicsPSO>> _PSOMap;
	std::unordered_map<std::string, std::shared_ptr<dx12lib::SamplerTexture2D>> _textureMap;
};
