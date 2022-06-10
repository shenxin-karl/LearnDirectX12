#pragma once
#include "dx12lib/dx12libStd.h"
#include "D3D/Shader/ShaderCommon.h"
#include "D3D/d3dutil.h"
#include "GameTimer/GameTimer.h"
#include "BaseApp/BaseApp.h"

using namespace Math;

struct ShapeVertex {
	float3 position;
	float3 normal;
	float2 texcoord;
};

struct SkullVertex {
	float3 position;
	float3 normal;
};

struct Mesh {
	std::shared_ptr<dx12lib::VertexBuffer> _pVertexBuffer;
	std::shared_ptr<dx12lib::IndexBuffer>  _pIndexBuffer;
};

struct ObjectCB {
	float4x4          world;
	d3d::Material	  material;
};

struct RenderItem {
	std::shared_ptr<Mesh> _pMesh;
	FRConstantBufferPtr<ObjectCB> _pObjectCB;
	std::shared_ptr<dx12lib::SamplerTexture2D> _pAlbedo;
};

struct Keyframe {
	Keyframe();
	~Keyframe();
public:
	float      timePoint;
	Vector3    translation;
	Vector3	   scale;
	Quaternion rotationQuat;
};

struct BoneAnimation {
	float getStartTime() const;
	float getEndTime() const;
	Matrix4 interpolate(float time) const;
	std::vector<Keyframe> keyframes;
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
	void buildRenderItem(dx12lib::DirectContextProxy pDirectCtx);
	void buildGeometry(dx12lib::DirectContextProxy pDirectCtx);
	void buildGameLight(dx12lib::DirectContextProxy pDirectCtx);
	void buildMaterials();
	void loadTextures(dx12lib::DirectContextProxy pDirectCtx);
	void renderShapesPass(dx12lib::DirectContextProxy pDirectCtx);
	void renderSkullPass(dx12lib::DirectContextProxy pDirectCtx);
	void pollEvent();
	void updatePassCB(std::shared_ptr<com::GameTimer> pGameTimer);
private:
	std::unique_ptr<d3d::SobelFilter> _pSobelFilter;
	std::unique_ptr<d3d::CoronaCamera>  _pCamera;
	FRConstantBufferPtr<d3d::CBLightType> _pGameLightsCB;
	FRConstantBufferPtr<d3d::CBPassType>  _pPassCB;
	std::unordered_map<std::string, d3d::Material> _materials;
	std::unordered_map<std::string, std::shared_ptr<Mesh>> _geometrys;
	std::unordered_map<std::string, std::vector<RenderItem>> _renderItems;
	std::unordered_map<std::string, std::shared_ptr<dx12lib::GraphicsPSO>> _PSOMap;
	std::unordered_map<std::string, std::shared_ptr<dx12lib::SamplerTexture2D>> _textureMap;
};
