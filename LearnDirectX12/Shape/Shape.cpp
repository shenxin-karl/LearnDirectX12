#include "Shape.h"
#include "D3D/d3dutil.h"
#include "D3D/Tool/Camera.h"
#include "D3D/Shader/ShaderCommon.h"
#include "D3D/Postprocessing/SobelFilter.h"
#include "D3D/dx12libHelper/RenderTarget.h"
#include "dx12lib/Texture/TextureStd.h"
#include "dx12lib/Context/ContextStd.h"
#include "dx12lib/Device/DeviceStd.h"
#include "dx12lib/Buffer/BufferStd.h"
#include "dx12lib/Pipeline/PipelineStd.h"
#include "Geometry/GeometryGenerator.h"
#include "InputSystem/InputSystem.h"
#include "InputSystem/Mouse.h"
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <iostream>

#include "D3D/Sky/SkyBox.h"
#include "D3D/M3dLoader/M3dLoader.h"
#include "D3D/Tool/Mesh.h"

Shape::Shape() {
	_title = "Shape";
}

Shape::~Shape() {
}

void Shape::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
	d3d::CameraDesc cameraDesc = {
		float3(10, 10, 10),
		float3(0, 1, 0),
		float3(0, 0, 0),
		45.f,
		0.1f,
		100.f,
		static_cast<float>(_width) / static_cast<float>(_height),
	};
	_pCamera = std::make_shared<d3d::CoronaCamera>(cameraDesc);
	_pCamera->_mouseWheelSensitivity = 1.f;
	_pPassCB = pDirectCtx->createFRConstantBuffer<d3d::CBPassType>();
	_pSkinnedBoneCb = pDirectCtx->createFRConstantBuffer<SkinnedBoneCB>();
	buildTexturePSO(pDirectCtx);
	buildColorPSO(pDirectCtx);
	buildSkinnedAnimationPSO(pDirectCtx);
	buildGameLight(pDirectCtx);
	buildGeometry(pDirectCtx);
	loadTextures(pDirectCtx);
	buildMaterials();
	buildSkullAnimation();
	loadModelAndBuildRenderItem(pDirectCtx);
	buildRenderItem(pDirectCtx);
	_pSobelFilter = std::make_unique<d3d::SobelFilter>(pDirectCtx, _width, _height);

	d3d::SkyBoxDesc skyBoxDesc;
	skyBoxDesc.pGraphicsCtx = pDirectCtx;
	skyBoxDesc.filename = L"resource/desertcube1024.dds";
	skyBoxDesc.renderTargetFormat = _pDevice->getDesc().backBufferFormat;
	skyBoxDesc.depthStencilFormat = _pDevice->getDesc().depthStencilFormat;
	_pSkyBox = std::make_unique<d3d::SkyBox>(skyBoxDesc);
}

void Shape::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	pollEvent();
	updatePassCB(pGameTimer);
	updateSkullAnimationCb(pGameTimer);
	updateSkinnedAnimationCb(pGameTimer);

	Vector3 dir { 1 };
	float radian = DirectX::XMConvertToRadians(pGameTimer->getTotalTime() * 20.f);
	Quaternion q { Vector3(0, 1, 0), radian };
	Vector3 d = q * dir;

	auto pLightCBVisitor = _pGameLightsCB->visit();
	pLightCBVisitor->lights[0].initAsDirectionLight(
		static_cast<float3>(d),
		float3(0.8f)
	);
}

void Shape::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue();
	auto pDirectCtx = pCmdQueue->createDirectContextProxy();
	{

		d3d::RenderTarget renderTarget(_pSwapChain);
		renderTarget.bind(pDirectCtx);
		renderTarget.clear(pDirectCtx, float4(DirectX::Colors::LightSkyBlue));
		renderShapesPass(pDirectCtx);
		renderSkullPass(pDirectCtx);
		renderSkinnedAnimationPass(pDirectCtx);
		_pSkyBox->render(pDirectCtx, _pCamera);
		//_pSobelFilter->apply(pDirectCtx, renderTarget.getRenderTarget2D());
		//pDirectCtx->copyResource(renderTarget.getRenderTarget2D(), _pSobelFilter->getOutput());
		renderTarget.unbind(pDirectCtx);
	}
	pCmdQueue->executeCommandList(pDirectCtx);
	pCmdQueue->signal(_pSwapChain);
}

void Shape::onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) {
	_pCamera->setAspect(float(width) / float(height));
	_pSobelFilter->onResize(pDirectCtx, width, height);
}

void Shape::buildTexturePSO(dx12lib::DirectContextProxy pDirectCtx) {
	auto pRootSignature = _pDevice->createRootSignature(2, 6);
	pRootSignature->initStaticSampler(0, d3d::getStaticSamplers());
	pRootSignature->at(0).initAsDescriptorTable({
		{ dx12lib::RegisterSlot::CBV0, 2 },
	});
	pRootSignature->at(1).initAsDescriptorTable({
		{ dx12lib::RegisterSlot::CBV2, 1 },
		{ dx12lib::RegisterSlot::SRV0, 2 },
	});
	pRootSignature->finalize();

	auto pPSO = _pDevice->createGraphicsPSO("TexturePSO");
	pPSO->setRootSignature(pRootSignature);
	pPSO->setRenderTargetFormat(
		_pSwapChain->getRenderTargetFormat(),
		_pSwapChain->getDepthStencilFormat()
	);

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		dx12lib::VInputLayoutDescHelper(&ShapeVertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&ShapeVertex::normal, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&ShapeVertex::texcoord, "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&ShapeVertex::tangent, "TANGENT", DXGI_FORMAT_R32G32B32_FLOAT),
	};
	pPSO->setInputLayout(inputLayout);
	pPSO->setVertexShader(d3d::compileShader(L"shader/texture.hlsl", nullptr, "VS", "vs_5_0"));
	pPSO->setPixelShader(d3d::compileShader(L"shader/texture.hlsl", nullptr, "PS", "ps_5_0"));
	pPSO->finalize();
	_PSOMap["TexturePSO"] = pPSO;
}

void Shape::buildColorPSO(dx12lib::DirectContextProxy pDirectCtx) {
	auto pRootSignature = _pDevice->createRootSignature(2, 6);
	pRootSignature->initStaticSampler(0, d3d::getStaticSamplers());
	pRootSignature->at(0).initAsDescriptorTable({
		{ dx12lib::RegisterSlot::CBV0, 2 },
		{ dx12lib::RegisterSlot::SRV0, 1 },
	});
	pRootSignature->at(1).initAsDescriptorTable({
		{ dx12lib::RegisterSlot::CBV2, 1 }
	});
	pRootSignature->finalize();

	auto pPSO = _pDevice->createGraphicsPSO("ColorPSO");
	pPSO->setRootSignature(pRootSignature);
	pPSO->setRenderTargetFormat(
		_pSwapChain->getRenderTargetFormat(),
		_pSwapChain->getDepthStencilFormat()
	);

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		dx12lib::VInputLayoutDescHelper(&SkullVertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&SkullVertex::normal, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT),
	};
	pPSO->setInputLayout(inputLayout);
	pPSO->setVertexShader(d3d::compileShader(L"shader/color.hlsl", nullptr, "VS", "vs_5_0"));
	pPSO->setPixelShader(d3d::compileShader(L"shader/color.hlsl", nullptr, "PS", "ps_5_0"));
	pPSO->finalize();
	_PSOMap["ColorPSO"] = pPSO;
}

void Shape::buildSkinnedAnimationPSO(dx12lib::DirectContextProxy pDirectCtx) {
	auto pRootSignature = _pDevice->createRootSignature(2, 6);
	pRootSignature->initStaticSampler(0, d3d::getStaticSamplers());
	pRootSignature->at(0).initAsDescriptorTable({
		{ dx12lib::RegisterSlot::CBV0, 2 },
	});
	pRootSignature->at(1).initAsDescriptorTable({
		{ dx12lib::RegisterSlot::CBV2, 2 },
		{ dx12lib::RegisterSlot::SRV0, 2 },
	});
	pRootSignature->finalize();

	auto pPSO = _pDevice->createGraphicsPSO("SkinnedAnimationPSO");
	pPSO->setRootSignature(pRootSignature);
	pPSO->setRenderTargetFormat(
		_pSwapChain->getRenderTargetFormat(),
		_pSwapChain->getDepthStencilFormat()
	);

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		dx12lib::VInputLayoutDescHelper(&d3d::SkinnedVertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&d3d::SkinnedVertex::normal, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&d3d::SkinnedVertex::texcoord, "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&d3d::SkinnedVertex::tangent, "TANGENT", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&d3d::SkinnedVertex::boneWeights, "BONEWEIGHTS", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&d3d::SkinnedVertex::boneIndices, "BONEINDICES", DXGI_FORMAT_R8G8B8A8_UINT),
	};

	D3D_SHADER_MACRO macros[] = {
		{ "SKINNED_ANIMATION", nullptr },
		{ nullptr, nullptr },
	};
	pPSO->setInputLayout(inputLayout);
	pPSO->setVertexShader(d3d::compileShader(L"shader/texture.hlsl", macros, "VS", "vs_5_0"));
	pPSO->setPixelShader(d3d::compileShader(L"shader/texture.hlsl", macros, "PS", "ps_5_0"));
	pPSO->finalize();

	_PSOMap["SkinnedAnimationPSO"] = pPSO;
}

void Shape::buildRenderItem(dx12lib::DirectContextProxy pDirectCtx) {
	RenderItem boxItem;
	ObjectCB boxObjCb;

	namespace DX = DirectX;

	// build Texture RenderItem
	constexpr const char *pTexturePSOName = "TexturePSO";
	auto &textureRenderItems = _renderItems[pTexturePSOName];

	Matrix4 matWorld = Matrix4::makeTranslation(0.f, 1.5f, 0.f) * Matrix4::makeScale(2.f);
	boxObjCb.material = _materials["boxMat"];
	boxObjCb.matWorld = float4x4(matWorld);
	boxObjCb.matNormal = float4x4(transpose(inverse(matWorld)));

	auto pMesh = _geometrys["box"];
	boxItem.subMesh = pMesh->getSubMesh();
	boxItem.pVertexBuffer = pMesh->getVertexBuffer();
	boxItem.pIndexBuffer = pMesh->getIndexBuffer();
	boxItem.pAlbedo = _textureMap["bricks.dds"];
	boxItem.pNormal = _textureMap["bricks_nmap.dds"];
	boxItem.pObjectCb = pDirectCtx->createFRConstantBuffer<ObjectCB>(boxObjCb);
	textureRenderItems.push_back(boxItem);

	RenderItem gridItem;
	ObjectCB gridObjCB;
	gridObjCB.material = _materials["gridMat"];
	gridObjCB.matWorld = float4x4::identity();
	gridObjCB.matNormal = float4x4(Matrix4::identity());
	gridObjCB.matTexCoord = float4x4(Matrix4::makeScale(10.f, 10.f, 1.f));

	pMesh = _geometrys["grid"];
	gridItem.subMesh = pMesh->getSubMesh();
	gridItem.pVertexBuffer = pMesh->getVertexBuffer();
	gridItem.pIndexBuffer = pMesh->getIndexBuffer();
	gridItem.pAlbedo = _textureMap["tile.dds"];
	gridItem.pNormal = _textureMap["tile_nmap.dds"];
	gridItem.pObjectCb = pDirectCtx->createFRConstantBuffer<ObjectCB>(gridObjCB);
	textureRenderItems.push_back(gridItem);

	for (std::size_t i = 0; i < 5; ++i) {
		RenderItem leftCylRItem;
		RenderItem rightCylRItem;
		RenderItem leftSphereRItem;
		RenderItem rightSphereRItem;

		ObjectCB leftCylObjCB;
		ObjectCB rightCylObjCB;
		ObjectCB leftSphereObjCB;
		ObjectCB rightSphereObjCB;

		Matrix4 matWorld0 = Matrix4::makeTranslation(-5.f, 1.5f, -10.f + i * 5.f);
		Matrix4 matWorld1 = Matrix4::makeTranslation(+5.f, 1.5f, -10.f + i * 5.f);
		Matrix4 matWorld2 = Matrix4::makeTranslation(-5.f, 3.5f, -10.f + i * 5.f);
		Matrix4 matWorld3 = Matrix4::makeTranslation(+5.f, 3.5f, -10.f + i * 5.f);

		leftCylObjCB.matWorld = float4x4(matWorld0);
		rightCylObjCB.matWorld = float4x4(matWorld1);
		leftSphereObjCB.matWorld = float4x4(matWorld2);
		rightSphereObjCB.matWorld = float4x4(matWorld3);

		leftCylObjCB.matNormal = float4x4(transpose(inverse(matWorld0)));
		rightCylObjCB.matNormal = float4x4(transpose(inverse(matWorld1)));
		leftSphereObjCB.matNormal = float4x4(transpose(inverse(matWorld2)));
		rightSphereObjCB.matNormal = float4x4(transpose(inverse(matWorld3)));

		leftCylObjCB.material = _materials["cylinderMat"];
		rightCylObjCB.material = _materials["cylinderMat"];
		leftSphereObjCB.material = _materials["sphereMat"];
		rightSphereObjCB.material = _materials["sphereMat"];

		auto pCylinderMesh = _geometrys["cylinder"];
		auto pShapeMesh = _geometrys["sphere"];
		leftCylRItem.subMesh = pCylinderMesh->getSubMesh();
		rightCylRItem.subMesh = pCylinderMesh->getSubMesh();
		leftSphereRItem.subMesh = pShapeMesh->getSubMesh();
		rightSphereRItem.subMesh = pShapeMesh->getSubMesh();

		leftCylRItem.pVertexBuffer = pCylinderMesh->getVertexBuffer();
		rightCylRItem.pVertexBuffer = pCylinderMesh->getVertexBuffer();
		leftSphereRItem.pVertexBuffer = pShapeMesh->getVertexBuffer();
		rightSphereRItem.pVertexBuffer = pShapeMesh->getVertexBuffer();

		leftCylRItem.pIndexBuffer = pCylinderMesh->getIndexBuffer();
		rightCylRItem.pIndexBuffer = pCylinderMesh->getIndexBuffer();
		leftSphereRItem.pIndexBuffer = pShapeMesh->getIndexBuffer();
		rightSphereRItem.pIndexBuffer = pShapeMesh->getIndexBuffer();

		leftCylRItem.pAlbedo = _textureMap["bricks2.dds"];
		rightCylRItem.pAlbedo = _textureMap["bricks2.dds"];
		leftSphereRItem.pAlbedo = _textureMap["bricks2.dds"];
		rightSphereRItem.pAlbedo = _textureMap["bricks2.dds"];

		leftCylRItem.pNormal = _textureMap["bricks2_nmap.dds"];
		rightCylRItem.pNormal = _textureMap["bricks2_nmap.dds"];
		leftSphereRItem.pNormal = _textureMap["bricks2_nmap.dds"];
		rightSphereRItem.pNormal = _textureMap["bricks2_nmap.dds"];

		leftCylRItem.pObjectCb = pDirectCtx->createFRConstantBuffer<ObjectCB>(leftCylObjCB);
		rightCylRItem.pObjectCb = pDirectCtx->createFRConstantBuffer<ObjectCB>(rightCylObjCB);
		leftSphereRItem.pObjectCb = pDirectCtx->createFRConstantBuffer<ObjectCB>(leftSphereObjCB);
		rightSphereRItem.pObjectCb = pDirectCtx->createFRConstantBuffer<ObjectCB>(rightSphereObjCB);

		textureRenderItems.push_back(leftCylRItem);
		textureRenderItems.push_back(rightCylRItem);
		textureRenderItems.push_back(leftSphereRItem);
		textureRenderItems.push_back(rightSphereRItem);
	}

	// build Color RenderItem
	constexpr const char *pColorPSOName = "ColorPSO";
	auto &colorRenderItems = _renderItems[pColorPSOName];
	RenderItem skullItem;
	ObjectCB skullObjCB;
	matWorld = Matrix4::makeTranslation(0.f, 1.f, 0.f);
	skullObjCB.material = _materials["skullMat"];
	skullObjCB.matWorld = float4x4(matWorld);
	skullObjCB.matNormal = float4x4(transpose(inverse(matWorld)));

	pMesh = _geometrys["skull"];
	skullItem.subMesh = pMesh->getSubMesh();
	skullItem.pVertexBuffer = pMesh->getVertexBuffer();
	skullItem.pIndexBuffer = pMesh->getIndexBuffer();
	skullItem.pObjectCb = pDirectCtx->createFRConstantBuffer<ObjectCB>(skullObjCB);
	colorRenderItems.push_back(skullItem);

	_skullMatWorld = skullObjCB.matWorld;
	_pSkullObjCB = skullItem.pObjectCb;
}

void Shape::buildGeometry(dx12lib::DirectContextProxy pDirectCtx) {
	const char * pStr= "%d";
	com::GometryGenerator gen;
	com::MeshData box = gen.createBox(1.5f, 1.5f, 1.5f, 3);
	com::MeshData grid = gen.createGrid(20.f, 30.f, 60, 40);
	com::MeshData sphere = gen.createCubeSphere(0.5f, 3);
	com::MeshData cylinder = gen.createCylinder(0.5f, 0.3f, 3.f, 20, 20);
	com::MeshData skull = gen.loadObjFile("resource/skull.obj");

	auto buildShapeMesh = [&](const com::MeshData &mesh) {
		std::vector<ShapeVertex> vertices;
		std::vector<std::uint16_t> indices;
		vertices.reserve(mesh.vertices.size());
		indices.reserve(mesh.indices.size());
		std::transform(mesh.vertices.begin(), mesh.vertices.end(), std::back_inserter(vertices), [](auto &v) {
			return ShapeVertex(v.position, v.normal, v.texcoord, v.tangent);
		});
		std::transform(mesh.indices.begin(), mesh.indices.end(), std::back_inserter(indices), [](auto &i) {
			return static_cast<std::uint16_t>(i);
		});

		auto pVertexBuffer = pDirectCtx->createVertexBuffer(
			vertices.data(),
			vertices.size(),
			sizeof(ShapeVertex)
		);

		auto pIndexBuffer = pDirectCtx->createIndexBuffer(
			indices.data(),
			indices.size(),
			DXGI_FORMAT_R16_UINT
		);
		return std::make_shared<d3d::Mesh>(pVertexBuffer, pIndexBuffer);;
	};

	auto buildSkullMesh = [&](const com::MeshData &mesh) {
		std::vector<SkullVertex> vertices;
		std::vector<std::uint16_t> indices;
		vertices.reserve(mesh.vertices.size());
		indices.reserve(mesh.indices.size());
		std::transform(mesh.vertices.begin(), mesh.vertices.end(), std::back_inserter(vertices), [](auto &v) {
			return SkullVertex(v.position, v.normal);
		});
		std::ranges::transform(mesh.indices, std::back_inserter(indices), [](auto &i) {
			return static_cast<std::uint16_t>(i);
		});

		auto pVertexBuffer = pDirectCtx->createVertexBuffer(
			vertices.data(),
			vertices.size(),
			sizeof(SkullVertex)
		);
		auto pIndexBuffer = pDirectCtx->createIndexBuffer(
			indices.data(),
			indices.size(),
			DXGI_FORMAT_R16_UINT
		);
		return std::make_shared<d3d::Mesh>(pVertexBuffer, pIndexBuffer);;
	};

	_geometrys["box"] = buildShapeMesh(box);
	_geometrys["grid"] = buildShapeMesh(grid);
	_geometrys["sphere"] = buildShapeMesh(sphere);
	_geometrys["cylinder"] = buildShapeMesh(cylinder);
	_geometrys["skull"] = buildSkullMesh(skull);
}


void Shape::buildGameLight(dx12lib::DirectContextProxy pDirectCtx) {
	_pGameLightsCB = pDirectCtx->createFRConstantBuffer<d3d::CBLightType>();
	auto pGPUGameLightCB = _pGameLightsCB->map();
	pGPUGameLightCB->directLightCount = 1;
	pGPUGameLightCB->pointLightCount = 1;
	pGPUGameLightCB->spotLightCount = 1;
	pGPUGameLightCB->ambientLight = float4(0.1f, 0.1f, 0.1f, 1.f);

	pGPUGameLightCB->lights[0].initAsDirectionLight(float3(-3, 6, -3), float3(0.8f));
	pGPUGameLightCB->lights[1].initAsPointLight(float3(0.f, 5.f, -10.f), float3(0.4f, 0.4f, 0.f), 0.f, 20.f);
	pGPUGameLightCB->lights[2].initAsSpotLight(
		float3(0.f, 20.f, 0.f), 
		float3(0.f, -1.f, 0.f),
		float3(0.f, 0.f, 1.f),
		0.f,
		50.f,
		32.f
	);
}

void Shape::buildMaterials() {
	namespace DX = DirectX;

	d3d::Material sphereMat;
	sphereMat.diffuseAlbedo = float4(DX::Colors::White);
	sphereMat.roughness = 0.5f;
	sphereMat.metallic = 0.5f;
	_materials["sphereMat"] = sphereMat;

	d3d::Material boxMat;
	boxMat.diffuseAlbedo = float4(DX::Colors::White);
	boxMat.roughness = 0.8f;
	boxMat.metallic = 0.2f;
	_materials["boxMat"] = boxMat;

	d3d::Material gridMat;
	gridMat.diffuseAlbedo = float4(DX::Colors::White);
	gridMat.roughness = 0.8f;
	gridMat.metallic = 0.2f;
	_materials["gridMat"] = gridMat;

	d3d::Material cylinderMat;
	cylinderMat.diffuseAlbedo = float4(DX::Colors::White);
	cylinderMat.roughness = 0.8f;
	cylinderMat.metallic = 0.2f;
	_materials["cylinderMat"] = cylinderMat;

	d3d::Material skullMat;
	//skullMat.diffuseAlbedo = float4(DX::Colors::Gold);
	skullMat.diffuseAlbedo = float4(DX::Colors::White);
	skullMat.roughness = 0.5f;
	skullMat.metallic = 0.5f;
	_materials["skullMat"] = skullMat;
}

void Shape::buildSkullAnimation() {
	Quaternion q0 { Vector3(0.f, 1.f, 0.f), DirectX::XMConvertToRadians(30.f) };
	Quaternion q1 { Vector3(1.f, 1.f, 2.f), DirectX::XMConvertToRadians(45.0f) };
	Quaternion q2 { Vector3(0.f, 1.f, 0.f), DirectX::XMConvertToRadians(-30.0f) };
	Quaternion q3 { Vector3(1.f, 0.f, 0.f), DirectX::XMConvertToRadians(70.0f) };

	constexpr float scale = 2.f;
	_skullAnimation.keyframes.resize(5);
	_skullAnimation.keyframes[0].timePoint = 0.f * scale;
	_skullAnimation.keyframes[0].translation = float3(-7.f, 0.f, 0.f);
	_skullAnimation.keyframes[0].scale = float3(0.5f);
	_skullAnimation.keyframes[0].rotationQuat = float4(q0);

	_skullAnimation.keyframes[1].timePoint = 2.f * scale;
	_skullAnimation.keyframes[1].translation = float3(0.f, 2.f, 10.f);
	_skullAnimation.keyframes[1].scale = float3(0.45f);
	_skullAnimation.keyframes[1].rotationQuat = float4(q1);

	_skullAnimation.keyframes[2].timePoint = 4.0f * scale;
	_skullAnimation.keyframes[2].translation = float3(7.f, 0.f, 0.f);
	_skullAnimation.keyframes[2].scale = float3(0.5f);
	_skullAnimation.keyframes[2].rotationQuat = float4(q2);

	_skullAnimation.keyframes[3].timePoint = 6.0f * scale;
	_skullAnimation.keyframes[3].translation = float3(0.0f, 1.0f, -10.0f);
	_skullAnimation.keyframes[3].scale = float3(0.65f);
	_skullAnimation.keyframes[3].rotationQuat = float4(q3);

	_skullAnimation.keyframes[4].timePoint = 8.0f * scale;
	_skullAnimation.keyframes[4].translation = float3(-7.0f, 0.0f, 0.0f);
	_skullAnimation.keyframes[4].scale = float3(0.5f);
	_skullAnimation.keyframes[4].rotationQuat = float4(q0);
}

void Shape::loadModelAndBuildRenderItem(dx12lib::DirectContextProxy pDirectCtx) {
	std::vector<d3d::SkinnedVertex> vertices;
	std::vector<uint16_t> indices;
	std::vector<d3d::M3dLoader::Subset> subsets;
	std::vector<d3d::M3dLoader::M3dMaterial> materials;
	if (!d3d::M3dLoader::loadM3d("resource/soldier.m3d", vertices, indices, subsets, materials, _skinnedData)) {
		assert(false);
		return;
	}

	auto loadTexture = [&](const std::string &name) {
		auto iter = _textureMap.find(name);
		if (iter != _textureMap.end())
			return iter->second;

		std::wstring wTextureName = std::to_wstring("resource/" + name);
		auto pTexture = pDirectCtx->createDDSTexture2DFromFile(wTextureName);
		assert(pTexture != nullptr);
		return pTexture;
	};


	Matrix4 scale = Matrix4::makeScale(0.08f);
	Matrix4 translation = Matrix4::makeTranslation(0.f, 0.f, -7.5f);
	Matrix4 world = translation * scale;
	float4x4 matWorld{ world };

	auto pVertexBuffer = pDirectCtx->createVertexBuffer(vertices.data(), vertices.size(), sizeof(d3d::SkinnedVertex));
	auto pIndexBuffer = pDirectCtx->createIndexBuffer(indices.data(), indices.size(), DXGI_FORMAT_R16_UINT);
	auto &renderItemQueue = _renderItems["SkinnedAnimationPSO"];
	for (size_t i = 0; i < subsets.size(); ++i) {
		const auto &subSet = subsets[i];
		const auto &material = materials[i];
		assert(subSet.id != -1);

		RenderItem rItem;
		ObjectCB objectCb;
		objectCb.matWorld = matWorld;
		objectCb.matNormal = float4x4::identity();
		objectCb.matTexCoord = float4x4::identity();
		objectCb.material.roughness = material.roughness;
		objectCb.material.diffuseAlbedo = material.diffuseAlbedo;
		objectCb.material.metallic = (material.fresnelR0.x + material.fresnelR0.y + material.fresnelR0.z) / 3.f;
		objectCb.material.metallic = std::clamp(objectCb.material.metallic, 0.f, 1.f);

		rItem.pObjectCb = pDirectCtx->createFRConstantBuffer<ObjectCB>(objectCb);
		rItem.subMesh.name = std::to_string(subSet.id);
		rItem.subMesh.count = subSet.faceCount * 3;
		rItem.subMesh.startIndexLocation = subSet.faceStart * 3;
		rItem.subMesh.baseVertexLocation = 0;

		rItem.pVertexBuffer = pVertexBuffer;
		rItem.pIndexBuffer = pIndexBuffer;

		rItem.pAlbedo = loadTexture(material.diffuseMapName);
		rItem.pNormal = loadTexture(material.normalMapName);
		renderItemQueue.push_back(rItem);
	}
}

void Shape::loadTextures(dx12lib::DirectContextProxy pDirectCtx) {
	_textureMap["bricks.dds"] = pDirectCtx->createDDSTexture2DFromFile(L"resource/bricks.dds");
	_textureMap["bricks_nmap.dds"] = pDirectCtx->createDDSTexture2DFromFile(L"resource/bricks_nmap.dds");
	_textureMap["tile.dds"] = pDirectCtx->createDDSTexture2DFromFile(L"resource/tile.dds");
	_textureMap["tile_nmap.dds"] = pDirectCtx->createDDSTexture2DFromFile(L"resource/tile_nmap.dds");
	_textureMap["bricks2.dds"] = pDirectCtx->createDDSTexture2DFromFile(L"resource/bricks2.dds");
	_textureMap["bricks2_nmap.dds"] = pDirectCtx->createDDSTexture2DFromFile(L"resource/bricks2_nmap.dds");
}

void Shape::renderShapesPass(dx12lib::DirectContextProxy pDirectCtx) {
	const std::string passPSOName = "TexturePSO";
	auto pPSO = _PSOMap[passPSOName];
	pDirectCtx->setGraphicsPSO(pPSO);

	pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV0, _pPassCB);
	pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV1, _pGameLightsCB);

	auto psoRenderItems = _renderItems[passPSOName];
	for (auto &rItem : psoRenderItems) {
		pDirectCtx->setVertexBuffer(rItem.pVertexBuffer);
		pDirectCtx->setIndexBuffer(rItem.pIndexBuffer);
		pDirectCtx->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV2, rItem.pObjectCb);
		pDirectCtx->setShaderResourceView(dx12lib::RegisterSlot::SRV0, rItem.pAlbedo->getSRV());
		pDirectCtx->setShaderResourceView(dx12lib::RegisterSlot::SRV1, rItem.pNormal->getSRV());
		rItem.subMesh.drawIndexedInstanced(pDirectCtx);
	}
}

void Shape::renderSkullPass(dx12lib::DirectContextProxy pDirectCtx) {
	const std::string passPSOName = "ColorPSO";
	auto pPSO = _PSOMap[passPSOName];

	pDirectCtx->setGraphicsPSO(pPSO);
	pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV0, _pPassCB);
	pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV1, _pGameLightsCB);
	pDirectCtx->setShaderResourceView(dx12lib::RegisterSlot::SRV0, _pSkyBox->getEnvironmentMap()->getSRV());

	auto psoRenderItems = _renderItems[passPSOName];
	auto &rItem = psoRenderItems[0];
	pDirectCtx->setVertexBuffer(rItem.pVertexBuffer);
	pDirectCtx->setIndexBuffer(rItem.pIndexBuffer);
	pDirectCtx->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV2, rItem.pObjectCb);
	rItem.subMesh.drawIndexedInstanced(pDirectCtx);
}

void Shape::renderSkinnedAnimationPass(dx12lib::DirectContextProxy pDirectCtx) {
	const std::string passPSOName = "SkinnedAnimationPSO";
	auto pso = _PSOMap[passPSOName];

	pDirectCtx->setGraphicsPSO(pso);
	pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV0, _pPassCB);
	pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV1, _pGameLightsCB);
	pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV3, _pSkinnedBoneCb);
	for (auto &rItem : _renderItems[passPSOName]) {
		pDirectCtx->setVertexBuffer(rItem.pVertexBuffer);
		pDirectCtx->setIndexBuffer(rItem.pIndexBuffer);
		pDirectCtx->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV2, rItem.pObjectCb);
		pDirectCtx->setShaderResourceView(dx12lib::RegisterSlot::SRV0, rItem.pAlbedo->getSRV());
		pDirectCtx->setShaderResourceView(dx12lib::RegisterSlot::SRV1, rItem.pNormal->getSRV());
		rItem.subMesh.drawIndexedInstanced(pDirectCtx);
	}
}

void Shape::pollEvent() {
	while (auto event = _pInputSystem->pMouse->getEvent())
		_pCamera->pollEvent(event);
}

void Shape::updatePassCB(std::shared_ptr<com::GameTimer> pGameTimer) {
	_pCamera->update(pGameTimer);
	auto pGPUPassCB = _pPassCB->map();

	_pCamera->updatePassCB(*pGPUPassCB);
	pGPUPassCB->renderTargetSize = _pSwapChain->getRenderTargetSize();
	pGPUPassCB->invRenderTargetSize = _pSwapChain->getInvRenderTargetSize();
	pGPUPassCB->totalTime = pGameTimer->getTotalTime();
	pGPUPassCB->deltaTime = pGameTimer->getDeltaTime();
}

void Shape::updateSkullAnimationCb(std::shared_ptr<com::GameTimer> pGameTimer) {
	_skullAnimationTimePoint += pGameTimer->getDeltaTime();
	if (_skullAnimationTimePoint > _skullAnimation.getEndTime())
		_skullAnimationTimePoint = 0.f;

	Matrix4 matWorld { _skullMatWorld };
	Matrix4 animationMatrix { _skullAnimation.interpolate(_skullAnimationTimePoint) };
	matWorld = animationMatrix * matWorld;
	auto pSkullCBVisitor = _pSkullObjCB->visit();
	pSkullCBVisitor->matWorld = float4x4(matWorld);
	pSkullCBVisitor->matNormal = float4x4(transpose(inverse(matWorld)));
}

void Shape::updateSkinnedAnimationCb(std::shared_ptr<com::GameTimer> pGameTimer) {
	_skinnedAnimationTimePoint += pGameTimer->getDeltaTime();
	if (_skinnedAnimationTimePoint > _skinnedData.getClipEndTime("Take1"))
		_skinnedAnimationTimePoint = 0.f;

	auto pSkinnedBoneCbVisit = _pSkinnedBoneCb->visit();
	std::vector<float4x4> boneTransforms = _skinnedData.getFinalTransforms("Take1", _skinnedAnimationTimePoint);
	size_t limit = std::min(SkinnedBoneCB::kMaxCount, boneTransforms.size());
	for (size_t i = 0; i < limit; ++i)
		pSkinnedBoneCbVisit->boneTransforms[i] = boneTransforms[i];
}
