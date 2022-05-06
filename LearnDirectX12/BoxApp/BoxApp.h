#pragma once
#include "BaseApp/BaseApp.h"
#include "D3D/Shader/ShaderCommon.h"
#include "D3D/Tool/Camera.h"
#include "GameTimer/GameTimer.h"
#include "Geometry/GeometryGenerator.h"

using namespace Math;

struct Vertex {
	float3 position;
	float3 normal;
public:
	Vertex(const com::Vertex &vertex) : position(vertex.position), normal(vertex.normal) {}
};

struct BoxMesh {
	std::shared_ptr<dx12lib::VertexBuffer> _pVertexBuffer;
	std::shared_ptr<dx12lib::IndexBuffer>  _pIndexBuffer;
	UINT _baseVertexLocation = 0;
	UINT _startIndexLocation = 0;
};

enum BoxRootParame : UINT {
	WorldViewProjCBuffer = 0,
};

struct CBObject {
	float4x4      gMatWorldViewProj;
	float4x4	  gMatNormal;
	d3d::Material gMaterial;
};

class BoxApp : public com::BaseApp {
public:
	BoxApp();
protected:
	void onInitialize(dx12lib::DirectContextProxy pDirectContext) override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void resize(int width, int height) override;
private:
	void pollEvent();
	void buildBoxGeometry(dx12lib::DirectContextProxy pDirectContext);
	void renderBoxPass(dx12lib::DirectContextProxy pDirectContext) const;
private:
	std::shared_ptr<dx12lib::GraphicsPSO>  _pGraphicsPSO;
	std::unique_ptr<d3d::CoronaCamera>     _pCamera;
	FRConstantBufferPtr<CBObject>		   _pCBObject;
	std::unique_ptr<BoxMesh>               _pBoxMesh;
	float    _theta = 0.f;
	float    _phi = 0.f;
	float    _radius = 5.f;
	bool     _isMouseLeftPress = false;
	POINT    _lastMousePosition;
};