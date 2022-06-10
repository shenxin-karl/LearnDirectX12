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

Keyframe::Keyframe() : timePoint(0.f), scale(1.f) {
}

float BoneAnimation::getStartTime() const {
	return keyframes.front().timePoint;
}

float BoneAnimation::getEndTime() const {
	return keyframes.back().timePoint;
}

Matrix4 BoneAnimation::interpolate(float time) const {
	assert(!keyframes.empty());
	Vector4 origin { 0.f, 0.f, 0.f, 1.f };
	if (time <= getStartTime()) {
		const auto &first = keyframes.front();
		return Matrix4(DirectX::XMMatrixAffineTransformation(
			first.scale,
			origin,
			first.rotationQuat,
			first.translation
		));
	}
	if (time >= getEndTime()) {
		const auto &back = keyframes.back();
		return Matrix4(DirectX::XMMatrixAffineTransformation(
			back.scale,
			origin,
			back.rotationQuat,
			back.translation
		));
	}
	for (size_t i = 0; i < keyframes.size()-1; ++i) {
		bool inside = (time >= keyframes[i].timePoint && time < keyframes[i+1].timePoint);
		if (!inside)
			continue;

		float t = (time - keyframes[i].timePoint) / (keyframes[i+1].timePoint - keyframes[i].timePoint);
		Vector3 scale = lerp(keyframes[i].scale, keyframes[i+1].scale, t);
		Vector3 translation = lerp(keyframes[i].translation, keyframes[i+1].translation, t);
		Quaternion rotate = slerp(keyframes[i].rotationQuat, keyframes[i+1].rotationQuat, t);
		return Matrix4(DirectX::XMMatrixAffineTransformation(
			scale,
			origin,
			rotate,
			translation
		));
	}
	assert(false);
	return Matrix4::identity();
}

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
	_pCamera = std::make_unique<d3d::CoronaCamera>(cameraDesc);
	_pCamera->_mouseWheelSensitivity = 1.f;
	_pPassCB = pDirectCtx->createFRConstantBuffer<d3d::CBPassType>();
	buildTexturePSO(pDirectCtx);
	buildColorPSO(pDirectCtx);
	buildGameLight(pDirectCtx);
	buildGeometry(pDirectCtx);
	loadTextures(pDirectCtx);
	buildMaterials();
	buildSkullAnimation();
	buildRenderItem(pDirectCtx);
	_pSobelFilter = std::make_unique<d3d::SobelFilter>(pDirectCtx, _width, _height);
}

void Shape::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	pollEvent();
	updatePassCB(pGameTimer);
	updateSkullAnimation(pGameTimer);
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
		{ dx12lib::RegisterSlot::SRV0, 1 },
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
	};
	pPSO->setInputLayout(inputLayout);
	pPSO->setVertexShader(d3d::compileShader(L"shader/texture.hlsl", nullptr, "VS", "vs_5_0"));
	pPSO->setPixelShader(d3d::compileShader(L"shader/texture.hlsl", nullptr, "PS", "ps_5_0"));
	pPSO->finalize();
	_PSOMap["TexturePSO"] = pPSO;
}

void Shape::buildColorPSO(dx12lib::DirectContextProxy pDirectCtx) {
	auto pRootSignature = _pDevice->createRootSignature(2);
	pRootSignature->at(0).initAsDescriptorTable({
		{ dx12lib::RegisterSlot::CBV0, 2 }
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

void Shape::buildRenderItem(dx12lib::DirectContextProxy pDirectCtx) {
	RenderItem boxItem;
	ObjectCB boxObjCb;

	namespace DX = DirectX;

	// build Texture RenderItem
	constexpr const char *pTexturePSOName = "TexturePSO";
	auto &textureRenderItems = _renderItems[pTexturePSOName];

	Matrix4 matWorld = Matrix4::makeTranslation(0.f, 0.5f, 0.f) * Matrix4::makeScale(2.f);
	boxObjCb.material = _materials["boxMat"];
	boxObjCb.matWorld = float4x4(matWorld);
	boxObjCb.matNormal = float4x4(transpose(inverse(matWorld)));
	boxItem._pMesh = _geometrys["box"];
	boxItem._pAlbedo = _textureMap["bricks.dds"];
	boxItem._pObjectCB = pDirectCtx->createFRConstantBuffer<ObjectCB>(boxObjCb);
	textureRenderItems.push_back(boxItem);

	RenderItem gridItem;
	ObjectCB gridObjCB;
	gridObjCB.material = _materials["gridMat"];
	gridObjCB.matWorld = MathHelper::identity4x4();;
	gridObjCB.matNormal = float4x4(Matrix4::identity());
	gridItem._pMesh = _geometrys["grid"];
	gridItem._pAlbedo = _textureMap["tile.dds"];
	gridItem._pObjectCB = pDirectCtx->createFRConstantBuffer<ObjectCB>(gridObjCB);
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

		leftCylRItem._pMesh = _geometrys["cylinder"];
		rightCylRItem._pMesh = _geometrys["cylinder"];
		leftSphereRItem._pMesh = _geometrys["sphere"];
		rightSphereRItem._pMesh = _geometrys["sphere"];

		leftCylRItem._pAlbedo = _textureMap["bricks.dds"];
		rightCylRItem._pAlbedo = _textureMap["bricks.dds"];
		leftSphereRItem._pAlbedo = _textureMap["bricks.dds"];
		rightSphereRItem._pAlbedo = _textureMap["bricks.dds"];

		leftCylRItem._pObjectCB = pDirectCtx->createFRConstantBuffer<ObjectCB>(leftCylObjCB);
		rightCylRItem._pObjectCB = pDirectCtx->createFRConstantBuffer<ObjectCB>(rightCylObjCB);
		leftSphereRItem._pObjectCB = pDirectCtx->createFRConstantBuffer<ObjectCB>(leftSphereObjCB);
		rightSphereRItem._pObjectCB = pDirectCtx->createFRConstantBuffer<ObjectCB>(rightSphereObjCB);

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
	skullItem._pMesh = _geometrys["skull"];
	skullItem._pObjectCB = pDirectCtx->createFRConstantBuffer<ObjectCB>(skullObjCB);
	colorRenderItems.push_back(skullItem);

	_skullMatWorld = skullObjCB.matWorld;
	_pSkullObjCB = skullItem._pObjectCB;
}

void Shape::buildGeometry(dx12lib::DirectContextProxy pDirectCtx) {
	com::GometryGenerator gen;
	com::MeshData box = gen.createBox(1.5f, 0.5f, 1.5f, 3);
	com::MeshData grid = gen.createGrid(20.f, 30.f, 60, 40);
	com::MeshData sphere = gen.createSphere(0.5f, 20, 20);
	com::MeshData cylinder = gen.createCylinder(0.5f, 0.3f, 3.f, 20, 20);
	com::MeshData skull = gen.loadObjFile("resource/skull.obj");

	auto buildShapeMesh = [&](const com::MeshData &mesh) {
		std::vector<ShapeVertex> vertices;
		std::vector<std::uint16_t> indices;
		vertices.reserve(mesh.vertices.size());
		indices.reserve(mesh.indices.size());
		std::transform(mesh.vertices.begin(), mesh.vertices.end(), std::back_inserter(vertices), [](auto &v) {
			return ShapeVertex(v.position, v.normal, v.texcoord);
		});
		std::transform(mesh.indices.begin(), mesh.indices.end(), std::back_inserter(indices), [](auto &i) {
			return static_cast<std::uint16_t>(i);
		});

		std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>();
		pMesh->_pVertexBuffer = pDirectCtx->createVertexBuffer(
			vertices.data(),
			vertices.size(),
			sizeof(ShapeVertex)
		);
		pMesh->_pIndexBuffer = pDirectCtx->createIndexBuffer(
			indices.data(),
			indices.size(),
			DXGI_FORMAT_R16_UINT
		);
		return pMesh;
	};

	auto buildSkullMesh = [&](const com::MeshData &mesh) {
		std::vector<SkullVertex> vertices;
		std::vector<std::uint16_t> indices;
		vertices.reserve(mesh.vertices.size());
		indices.reserve(mesh.indices.size());
		std::transform(mesh.vertices.begin(), mesh.vertices.end(), std::back_inserter(vertices), [](auto &v) {
			return SkullVertex(v.position, v.normal);
		});
		std::transform(mesh.indices.begin(), mesh.indices.end(), std::back_inserter(indices), [](auto &i) {
			return static_cast<std::uint16_t>(i);
		});
		std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>();
		pMesh->_pVertexBuffer = pDirectCtx->createVertexBuffer(
			vertices.data(),
			vertices.size(),
			sizeof(SkullVertex)
		);
		pMesh->_pIndexBuffer = pDirectCtx->createIndexBuffer(
			indices.data(),
			indices.size(),
			DXGI_FORMAT_R16_UINT
		);
		return pMesh;
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
	pGPUGameLightCB->spotLightCount = 0;
	pGPUGameLightCB->ambientLight = float4(0.1f, 0.1f, 0.1f, 1.f);

	pGPUGameLightCB->lights[0].initAsDirectionLight(float3(-3, 6, -3), float3(1.f));
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
	boxMat.roughness = 1.0f;
	boxMat.metallic = 0.f;
	_materials["boxMat"] = boxMat;

	d3d::Material gridMat;
	gridMat.diffuseAlbedo = float4(DX::Colors::White);
	gridMat.roughness = 1.f;
	gridMat.metallic = 0.1f;
	_materials["gridMat"] = gridMat;

	d3d::Material cylinderMat;
	cylinderMat.diffuseAlbedo = float4(DX::Colors::White);
	cylinderMat.roughness = 1.f;
	cylinderMat.metallic = 0.f;
	_materials["cylinderMat"] = cylinderMat;

	d3d::Material skullMat;
	skullMat.diffuseAlbedo = float4(DX::Colors::Gold);
	skullMat.roughness = 0.8f;
	skullMat.metallic = 0.2f;
	_materials["skullMat"] = skullMat;
}

void Shape::buildSkullAnimation() {
	Quaternion q0 { Vector3(0.f, 1.f, 0.f), DirectX::XMConvertToRadians(30.f) };
	Quaternion q1 { Vector3(1.f, 1.f, 2.f), DirectX::XMConvertToRadians(45.0f) };
	Quaternion q2 { Vector3(0.f, 1.f, 0.f), DirectX::XMConvertToRadians(-30.0f) };
	Quaternion q3 { Vector3(1.f, 0.f, 0.f), DirectX::XMConvertToRadians(70.0f) };

	_skullAnimation.keyframes.resize(5);
	_skullAnimation.keyframes[0].timePoint = 0.f;
	_skullAnimation.keyframes[0].translation = Vector3(-7.f, 0.f, 0.f);
	_skullAnimation.keyframes[0].scale = Vector3(0.5f);
	_skullAnimation.keyframes[0].rotationQuat = q0;

	_skullAnimation.keyframes[1].timePoint = 2.f;
	_skullAnimation.keyframes[1].translation = Vector3(0.f, 2.f, 10.f);
	_skullAnimation.keyframes[1].scale = Vector3(0.5f);
	_skullAnimation.keyframes[1].rotationQuat = q1;

	_skullAnimation.keyframes[2].timePoint = 4.0f;
	_skullAnimation.keyframes[2].translation = Vector3(7.f, 0.f, 0.f);
	_skullAnimation.keyframes[2].scale = Vector3(0.5f);
	_skullAnimation.keyframes[2].rotationQuat = q2;

	_skullAnimation.keyframes[3].timePoint = 6.0f;
	_skullAnimation.keyframes[3].translation = Vector3(0.0f, 1.0f, -10.0f);
	_skullAnimation.keyframes[3].scale = Vector3(0.5f);
	_skullAnimation.keyframes[3].rotationQuat = q3;

	_skullAnimation.keyframes[4].timePoint = 8.0f;
	_skullAnimation.keyframes[4].translation = Vector3(-7.0f, 0.0f, 0.0f);
	_skullAnimation.keyframes[4].scale = Vector3(0.5f);
	_skullAnimation.keyframes[4].rotationQuat = q0;
}

void Shape::loadTextures(dx12lib::DirectContextProxy pDirectCtx) {
	_textureMap["bricks.dds"] = pDirectCtx->createDDSTexture2DFromFile(L"resource/bricks.dds");
	_textureMap["tile.dds"] = pDirectCtx->createDDSTexture2DFromFile(L"resource/tile.dds");
}

void Shape::renderShapesPass(dx12lib::DirectContextProxy pDirectCtx) {
	const std::string passPSOName = "TexturePSO";
	auto pPSO = _PSOMap[passPSOName];
	pDirectCtx->setGraphicsPSO(pPSO);

	pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV0, _pPassCB);
	pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV1, _pGameLightsCB);

	auto psoRenderItems = _renderItems[passPSOName];
	for (auto &rItem : psoRenderItems) {
		pDirectCtx->setVertexBuffer(rItem._pMesh->_pVertexBuffer);
		pDirectCtx->setIndexBuffer(rItem._pMesh->_pIndexBuffer);
		pDirectCtx->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV2, rItem._pObjectCB);
		pDirectCtx->setShaderResourceView(dx12lib::RegisterSlot::SRV0, rItem._pAlbedo->getSRV());
		pDirectCtx->drawIndexedInstanced(
			rItem._pMesh->_pIndexBuffer->getIndexCount(), 1, 0,
			0, 0
		);
	}
}

void Shape::renderSkullPass(dx12lib::DirectContextProxy pDirectCtx) {
	const std::string passPSOName = "ColorPSO";
	auto pPSO = _PSOMap[passPSOName];

	pDirectCtx->setGraphicsPSO(pPSO);
	pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV0, _pPassCB);
	pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV1, _pGameLightsCB);
	auto psoRenderItems = _renderItems[passPSOName];
	auto &rItem = psoRenderItems[0];
	pDirectCtx->setVertexBuffer(rItem._pMesh->_pVertexBuffer);
	pDirectCtx->setIndexBuffer(rItem._pMesh->_pIndexBuffer);
	pDirectCtx->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDirectCtx->setConstantBuffer(dx12lib::RegisterSlot::CBV2, rItem._pObjectCB);
	pDirectCtx->drawIndexedInstanced(
		rItem._pMesh->_pIndexBuffer->getIndexCount(), 1, 0,
		0, 0
	);
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

void Shape::updateSkullAnimation(std::shared_ptr<com::GameTimer> pGameTimer) {
	_animationTimePoint += pGameTimer->getDeltaTime();
	if (_animationTimePoint > _skullAnimation.getEndTime())
		_animationTimePoint = 0.f;

	float4 color {
		std::sin(pGameTimer->getTotalTime()) * 0.5f + 0.5f,
		0.3f,
		std::cos(pGameTimer->getTotalTime()) * 0.5f + 0.5f,
		1.f
	};

	Matrix4 matWorld { _skullMatWorld };
	Matrix4 animationMatrix = _skullAnimation.interpolate(_animationTimePoint);
	matWorld = animationMatrix * matWorld;
	auto pSkullCBVisitor = _pSkullObjCB->visit();
	pSkullCBVisitor->matWorld = float4x4(matWorld);
	pSkullCBVisitor->matNormal = float4x4(transpose(inverse(matWorld)));
	pSkullCBVisitor->material.diffuseAlbedo = color;
}
