#pragma once
#include "BaseApp/BaseApp.h"
#include "D3D/Shader/ShaderCommon.h"
#include "D3D/Sky/SphericalHarmonics.hpp"
#include "D3D/Tool/Camera.h"
#include "GameTimer/GameTimer.h"
#include "Geometry/GeometryGenerator.h"
#include "Pipeline/ShaderRegister.hpp"

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

struct CBObject {
	float4x4          gMatWorld;
	float4x4	      gMatNormal;
	d3d::MaterialData gMaterial;
	d3d::SH3	      gSH3;
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
	std::shared_ptr<d3d::IBL>			   _pIBL;
	std::shared_ptr<d3d::SkyBox>		   _pSkyBox;
	std::shared_ptr<dx12lib::GraphicsPSO>  _pGraphicsPSO;
	std::shared_ptr<d3d::CoronaCamera>     _pCamera;
	FRConstantBufferPtr<CBObject>		   _pCBObject;
	FRConstantBufferPtr<d3d::CBPassType>   _pCBPass;
	std::unique_ptr<BoxMesh>               _pBoxMesh;
};