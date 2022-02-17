#include "Shape.h"
#include "D3D/d3dutil.h"
#include "D3D/Camera.h"
#include "D3D/ShaderCommon.h"
#include "dx12lib/Device.h"
#include "dx12lib/SwapChain.h"
#include "dx12lib/CommandList.h"
#include "dx12lib/PipelineStateObject.h"
#include "dx12lib/SwapChain.h"
#include "dx12lib/RenderTarget.h"
#include "dx12lib/Texture.h"
#include "dx12lib/IndexBuffer.h"
#include "dx12lib/RootSignature.h"
#include "Geometry/GeometryGenerator.h"

Shape::Shape() {
	_title = "Shape";
}

Shape::~Shape() {

}

void Shape::onInitialize(dx12lib::CommandListProxy pCmdList) {
	d3dutil::CameraDesc cameraDesc = {
		float3(3, 3, 3),
		float3(0, 1, 0),
		float3(0, 0, 0),
		45.f,
		0.1f,
		100.f,
		float(_width) / float(_height),
	};
	_pCamera = std::make_unique<d3dutil::CoronaCamera>(cameraDesc);
	_pPassCB = pCmdList->createStructConstantBuffer<d3dutil::PassCBType>();
	_pGameLightsCB = pCmdList->createStructConstantBuffer<d3dutil::LightCBType>();
	buildPSO(pCmdList);
	buildRenderItem(pCmdList);
}

void Shape::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {

}

void Shape::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {

}

void Shape::buildPSO(dx12lib::CommandListProxy pCmdList) {
	dx12lib::RootSignatureDescHelper rootDesc;
	rootDesc.reset(3);
	rootDesc[0].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	rootDesc[1].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	rootDesc[2].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	auto pRootSignature = _pDevice->createRootSignature(rootDesc);

	_pGraphicsPSO = _pDevice->createGraphicsPSO("ColorPSO");
	_pGraphicsPSO->setRootSignature(pRootSignature);
	_pGraphicsPSO->setRenderTargetFormat(
		_pSwapChain->getRenderTargetFormat(),
		_pSwapChain->getDepthStencilFormat(),
		_pDevice->getSampleCount(),
		_pDevice->getSampleQuality()
	);

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		dx12lib::VInputLayoutDescHelper(&ShapeVertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&ShapeVertex::normal, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT),
	};
	_pGraphicsPSO->setInputLayout(inputLayout);
	_pGraphicsPSO->setVertexShader(d3dutil::compileShader(L"shader/color.hlsl", nullptr, "VS", "vs_5_0"));
	_pGraphicsPSO->setPixelShader(d3dutil::compileShader(L"shader/color.hlsl", nullptr, "PS", "ps_5_0"));
	_pGraphicsPSO->finalize(_pDevice);
}

void Shape::buildRenderItem(dx12lib::CommandListProxy pCmdList) {
	
}

void Shape::buildGeometry(dx12lib::CommandListProxy pCmdList) {
	com::GometryGenerator gen;
	com::MeshData box = gen.createBox(1.5f, 0.5f, 1.5f, 3);
	com::MeshData grid = gen.createGrid(20.f, 30.f, 60, 40);
	com::MeshData sphere = gen.createSphere(0.5f, 3);
	com::MeshData cylinder = gen.createCylinder(0.5f, 0.3f, 3.f, 20, 20);

	auto buildShapeMesh = [&](const com::MeshData &mesh) {
		std::vector<ShapeVertex> vertices;
		std::vector<std::uint16_t> indices;
		vertices.reserve(mesh.vertices.size());
		indices.reserve(mesh.indices.size());
		std::transform(mesh.vertices.begin(), mesh.vertices.end(), std::back_inserter(vertices), [](auto &v) {
			return ShapeVertex(v.position, v.normal);
		});
		std::transform(mesh.indices.begin(), mesh.indices.end(), std::back_inserter(indices), [](auto &i) {
			return static_cast<std::uint16_t>(i);
		});

		// build std::shared_ptr<Mesh>
		std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>();
		pMesh->_pVertexBuffer = pCmdList->createVertexBuffer(
			vertices.data(),
			sizeof(ShapeVertex) * vertices.size(),
			sizeof(ShapeVertex)
		);
		pMesh->_pIndexBuffer = pCmdList->createIndexBuffer(
			indices.data(),
			sizeof(std::uint16_t) * indices.size(),
			DXGI_FORMAT_R16_UINT
		);
		return pMesh;
	};

	_geometrys["box"] = buildShapeMesh(box);
	_geometrys["grid"] = buildShapeMesh(grid);
	_geometrys["sphere"] = buildShapeMesh(sphere);
	_geometrys["cylinder"] = buildShapeMesh(cylinder);
}

void Shape::buildMaterials() {
	d3dutil::Material sphereMat;
	sphereMat.diffuseAlbedo = float3(DX::Colors::Red);
	sphereMat.fresnelR0 = float3(0.56f, 0.57f, 0.58f);
	sphereMat.roughness = 0.f;
	sphereMat.metallic = 1.f;
	_materials["sphereMat"] = sphereMat;

	d3dutil::Material boxMat;
	boxMat.diffuseAlbedo = float3(DX::Colors::Green);
	boxMat.fresnelR0 = float3(0.56f, 0.57f, 0.58f);
	boxMat.roughness = 0.f;
	boxMat.metallic = 0.2f;
	_materials["boxMat"] = boxMat;

	d3dutil::Material gridMat;
	gridMat.diffuseAlbedo = float3(DX::Colors::Gray);
	gridMat.fresnelR0 = float3(0.56f, 0.57f, 0.58f);
	gridMat.roughness = 1.f;
	gridMat.metallic = 0.f;
	_materials["gridMat"] = gridMat;

	d3dutil::Material cylinderMat;
	cylinderMat.diffuseAlbedo = float3(DX::Colors::LightSkyBlue);
	cylinderMat.fresnelR0 = float3(0.56f, 0.57f, 0.58f);
	cylinderMat.roughness = 0.5f;
	cylinderMat.metallic = 0.5f;
	_materials["cylinderMat"] = cylinderMat;
}

void Shape::renderShapesPass(dx12lib::CommandListProxy pCmdList) {

}

void Shape::pollEvent() {

}
