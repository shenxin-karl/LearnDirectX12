#include <DirectXMath.h>
#include <DirectXColors.h>
#include <string_view>
#include "D3DApp.h"
#include "Mouse.h"
#include "TestGraphics.h"
#include "D3DApp.h"
#include "window.h"

bool TestGraphics::initialize() {
	if (!Base::initialize())
		return false;

	ThrowIfFailed(commandList_->Reset(directCmdListAlloc_.Get(), nullptr));

	buildDescriptorHeaps();
	buildConstantBuffers();
	buildRootSignature();
	buildShaderAndInputLayout();
	buildBoxGeometry();
	buildPSO();

	ThrowIfFailed(commandList_->Close());
	ID3D12CommandList *cmdLists[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(1, cmdLists);
	flushCommandQueue();

	updateProj();
	return true;
}

void TestGraphics::tick(GameTimer &dt) {
	if (D3DApp::instance()->getWindow()->isPause())
		return;
	Base::tick(dt);
	handleMouseMsg();
	updateView();
	draw();
}

void TestGraphics::update() {
	Base::update();
}

void TestGraphics::draw() {
	ThrowIfFailed(directCmdListAlloc_->Reset());
	ThrowIfFailed(commandList_->Reset(directCmdListAlloc_.Get(), pso_.Get()));
	commandList_->RSSetViewports(1, &screenViewport_);
	commandList_->RSSetScissorRects(1, &scissiorRect_);
	commandList_->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
		currentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET))
	);
	commandList_->ClearRenderTargetView(currentBackBufferView(), DX::Colors::LightBlue, 0, nullptr);
	commandList_->ClearDepthStencilView(depthStencilView(), 
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	commandList_->OMSetRenderTargets(1, RVPtr(currentBackBufferView()), true, RVPtr(depthStencilView()));

	ID3D12DescriptorHeap *descriptorHeaps[] = { cbvHeap_.Get() };
	commandList_->SetDescriptorHeaps(static_cast<UINT>(std::size(descriptorHeaps)), descriptorHeaps);
	commandList_->SetGraphicsRootSignature(rootSignature_.Get());
	commandList_->IASetVertexBuffers(0, 1, RVPtr(objectGeometry_->vertexBufferView()));
	commandList_->IASetIndexBuffer(RVPtr(objectGeometry_->indexBufferView()));
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList_->SetGraphicsRootDescriptorTable(0, cbvHeap_->GetGPUDescriptorHandleForHeapStart());

	// update gWorldViewProj;
	DX::XMMATRIX modelMatrix = DX::XMLoadFloat4x4(&model_);
	DX::XMMATRIX viewMatrix = DX::XMLoadFloat4x4(&view_);
	DX::XMMATRIX projMatrix = DX::XMLoadFloat4x4(&project_);
	DX::XMMATRIX gWorldViewProj = modelMatrix * viewMatrix * projMatrix;
	BoxObjectConstant constant;
	DX::XMStoreFloat4x4(&constant.gWorldViewProj, DX::XMMatrixTranspose(gWorldViewProj));
	objectCB_->copyData(0, constant);

	commandList_->DrawIndexedInstanced(objectGeometry_->drawArgs["box"].indexCount, 1, 0, 0, 0);
	
	commandList_->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
		currentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT))
	);

	ThrowIfFailed(commandList_->Close());
	ID3D12CommandList *cmdLists[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(static_cast<UINT>(std::size(cmdLists)), cmdLists);

	ThrowIfFailed(swapChain_->Present(0, 0));
	currBackBuffer_ = (currBackBuffer_ + 1) % kSwapChainCount;
	
	flushCommandQueue();
}

void TestGraphics::onResize() {
	Base::onResize();
	updateProj();
}

void TestGraphics::handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	Base::handleMsg(hwnd, msg, wParam, lParam);
}


void TestGraphics::handleMouseMsg() {
	auto event = D3DApp::instance()->getMouse()->getEvent();
	if (event.isInvalid())
		return;

	POINT p = { event.x, event.y };
	switch (event.state_) {
	case Mouse::State::Move:
		onMouseMove(p);
		break;
	case Mouse::State::LPress:
		onMouseLPress(p);
		break;
	case Mouse::State::LRelease:
		onMouseLRelese(p);
		break;
	default:
		break;
	}
}


void TestGraphics::onMouseMove(POINT p) {
	if (!isMouseLPress_)
		return;

	using namespace DirectX;
	float dx = (p.x - lastMousePos_.x) * 0.5f;
	float dy = (p.y - lastMousePos_.y) * 0.5f;
	lastMousePos_ = p;

	phi_ = std::clamp(phi_ + dy, -89.f, 89.f);
	theta_ -= dx;
	std::string msg = std::format("phi: {}, theta: {}\n", phi_, theta_);
	OutputDebugStringA(msg.c_str());
}


void TestGraphics::onMouseLPress(POINT p) {
	isMouseLPress_ = true;
	lastMousePos_ = p;
}


void TestGraphics::onMouseLRelese(POINT p) {
	isMouseLPress_ = false;
}

void TestGraphics::buildDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(d3dDevice_->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbvHeap_)));
}

void TestGraphics::buildConstantBuffers() {
	objectCB_ = std::make_unique<UploadBuffer<BoxObjectConstant>>(d3dDevice_.Get(), 1, true);
	UINT objCBByteSize = calcConstantBufferByteSize(sizeof(BoxObjectConstant));
	auto cbAddress = objectCB_->resource()->GetGPUVirtualAddress();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = objCBByteSize;

	d3dDevice_->CreateConstantBufferView(&cbvDesc, cbvHeap_->GetCPUDescriptorHandleForHeapStart());
}

void TestGraphics::buildRootSignature() {
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc = {
		1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	};

	WRL::ComPtr<ID3DBlob> serizlizedRootSig = nullptr;
	WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&serizlizedRootSig,
		&errorBlob
	);

	if (FAILED(hr)) {
		OutputDebugStringA(reinterpret_cast<const char *>(errorBlob->GetBufferPointer()));
		ThrowIfFailed(hr);
	}

	ThrowIfFailed(d3dDevice_->CreateRootSignature(
		0,
		serizlizedRootSig->GetBufferPointer(),
		serizlizedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_)
	));
}

void TestGraphics::buildShaderAndInputLayout() {
	std::wstring hlslPath = L"shader/box/box.hlsl";
	vsByteCode_ = compileShader(hlslPath, nullptr, "VS", "vs_5_0");
	psByteCode_ = compileShader(hlslPath, nullptr, "PS", "ps_5_0");
	assert(vsByteCode_ != nullptr);
	assert(psByteCode_ != nullptr);

	const auto slotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	const int instanced = 0;
	inputLayout_ =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void TestGraphics::buildBoxGeometry() {
#if 1
	std::vector<BoxVertex> vertices = {
		{ DX::XMFLOAT3(-1.f, -1.f, -1.f), DX::XMFLOAT4(DX::Colors::White)   },
		{ DX::XMFLOAT3(-1.f, +1.f, -1.f), DX::XMFLOAT4(DX::Colors::Black)   },
		{ DX::XMFLOAT3(+1.f, +1.f, -1.f), DX::XMFLOAT4(DX::Colors::Red)     },
		{ DX::XMFLOAT3(+1.f, -1.f, -1.f), DX::XMFLOAT4(DX::Colors::Green)   },
		{ DX::XMFLOAT3(-1.f, -1.f, +1.f), DX::XMFLOAT4(DX::Colors::Blue)    },
		{ DX::XMFLOAT3(-1.f, +1.f, +1.f), DX::XMFLOAT4(DX::Colors::Yellow)  },
		{ DX::XMFLOAT3(+1.f, +1.f, +1.f), DX::XMFLOAT4(DX::Colors::Cyan)    },
		{ DX::XMFLOAT3(+1.f, -1.f, +1.f), DX::XMFLOAT4(DX::Colors::Magenta) },
	};

	std::vector<uint16_t> indices = {
		0, 1, 2,	// front
		0, 2, 3,
		4, 6, 5,	// back
		4, 7, 6,
		4, 5, 1,	// left
		4, 1, 0,
		3, 2, 6,	// right
		3, 6, 7,
		1, 5, 6,	// top
		1, 6, 2,
		4, 0, 3,	// bottom
		4, 3, 7,
	};
#else
	std::vector<BoxVertex> vertices = {
		{ DX::XMFLOAT3(-0.5f, -0.5f, +0.0f), DX::XMFLOAT4(DX::Colors::Red)   },
		{ DX::XMFLOAT3(+0.0f, +0.5f, +0.0f), DX::XMFLOAT4(DX::Colors::Green) },
		{ DX::XMFLOAT3(+0.5f, -0.5f, +0.0f), DX::XMFLOAT4(DX::Colors::Blue)  },
	};

	std::vector<int16_t> indices = {
		0, 1, 2
	};
#endif

	size_t vbByteSize = sizeof(BoxVertex) * vertices.size();
	size_t ibByteSize = sizeof(uint16_t) * indices.size();

	objectGeometry_ = std::make_unique<MeshGeometry>();
	objectGeometry_->name = "BoxGeometry";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &objectGeometry_->vertexBufferCPU));
	CopyMemory(objectGeometry_->vertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &objectGeometry_->indexBufferCPU));
	CopyMemory(objectGeometry_->indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	objectGeometry_->vertexBufferGPU = createDefaultBuffer(
		d3dDevice_.Get(),
		commandList_.Get(),
		vertices.data(),
		vbByteSize,
		objectGeometry_->vertexBufferUploader
	);

	objectGeometry_->indexBufferGPU = createDefaultBuffer(
		d3dDevice_.Get(),
		commandList_.Get(),
		indices.data(),
		ibByteSize,
		objectGeometry_->indexBufferUploader
	);

	objectGeometry_->vertexBufferByteSize = vbByteSize;
	objectGeometry_->indexBufferByteSize = ibByteSize;
	objectGeometry_->vertexByteStride = sizeof(BoxVertex);
	objectGeometry_->indexBufferFormat = DXGI_FORMAT_R16_UINT;

	SubmeshGeometry submesh;
	submesh.indexCount = indices.size();
	submesh.baseVertexLocation = 0;
	submesh.startIndexLocation = 0;

	objectGeometry_->drawArgs["box"] = submesh;
}

void TestGraphics::buildPSO() {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { inputLayout_.data(), (UINT)inputLayout_.size() };
	psoDesc.pRootSignature = rootSignature_.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(vsByteCode_->GetBufferPointer()),
		vsByteCode_->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(psByteCode_->GetBufferPointer()),
		psByteCode_->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = backBufferFormat_;
	psoDesc.SampleDesc.Count = getSampleCount();
	psoDesc.SampleDesc.Quality = getSampleQuality();
	psoDesc.DSVFormat = depthStencilFormat_;
	ThrowIfFailed(d3dDevice_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso_)));
}

void TestGraphics::updateView() {
	float radianTheta = theta_ / 180.f * DX::XM_PI;
	float radianPhi = phi_ / 180.f * DX::XM_PI;
	DX::XMFLOAT3 dir = {
		radius_ * std::cos(radianPhi) * std::cos(radianTheta),
		radius_ * std::sin(radianPhi),
		radius_ * std::cos(radianPhi) * std::sin(radianTheta),
	};

	using namespace DirectX;
	DX::XMVECTOR worldUp = DX::XMVectorSet(0.f, 1.f, 0.f, 0.f);
	DX::XMVECTOR lookFrom = DX::XMVectorSet(dir.x, dir.y, dir.z, 1.0f);
	DX::XMVECTOR lookAt = DX::XMVectorSet(0.f, 0.f, 0.f, 1.f);
	DX::XMFLOAT3 lk;
	DX::XMStoreFloat3(&lk, lookFrom);
	//std::string msg = std::format("lookform({}, {}, {})\n", lk.x, lk.y, lk.z);
	//OutputDebugStringA(msg.c_str());
	DX::XMMATRIX viewMatrix = DX::XMMatrixLookAtLH(lookFrom, lookAt, worldUp);
	DX::XMStoreFloat4x4(&view_, viewMatrix);
}

void TestGraphics::updateProj() {
	float aspect = D3DApp::instance()->getWindow()->aspectRatio();
	float fovAngle = 45.f / 180.f * DX::XM_PI;
	float nearPlane = 1.f;
	float farPlane = 1000.f;
	auto projMatrix = DX::XMMatrixPerspectiveFovLH(fovAngle, aspect, nearPlane, farPlane);
	DX::XMStoreFloat4x4(&project_, projMatrix);
}
