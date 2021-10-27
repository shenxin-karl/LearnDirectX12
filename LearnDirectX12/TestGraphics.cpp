#include <DirectXMath.h>
#include <DirectXColors.h>
#include <string_view>
#include "TestGraphics.h"
#include "D3DApp.h"
#include "window.h"

bool TestGraphics::initialize() {
	if (!Base::initialize())
		return false;

	buildDescriptorHeaps();
	buildRootSignature();
	buildShaderAndInputLayout();
	buildBoxGeometry();
	buildPSO();
	return true;
}

void TestGraphics::tick(GameTimer &dt) {
	if (D3DApp::instance()->getWindow()->isPause())
		return;
	Base::tick(dt);
	draw();
}

void TestGraphics::update() {
	Base::update();
}

void TestGraphics::draw() {
	//ThrowIfFailed(directCmdListAlloc_->Reset());
	//ThrowIfFailed(commandList_->Reset(
	//	directCmdListAlloc_.Get(), nullptr
	//));

	//commandList_->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
	//	currentBackBuffer(),
	//	D3D12_RESOURCE_STATE_PRESENT,
	//	D3D12_RESOURCE_STATE_RENDER_TARGET
	//)));

	//commandList_->RSSetViewports(1, &screenViewport_);
	//commandList_->RSSetScissorRects(1, &scissiorRect_);

	//// clear back target view and depth and stencil buffer
	//commandList_->ClearRenderTargetView(
	//	currentBackBufferView(),
	//	DirectX::Colors::LightSteelBlue,
	//	0,
	//	nullptr
	//);
	//commandList_->ClearDepthStencilView(
	//	depthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr
	//);

	//auto currentBackBufferViewObj = currentBackBufferView();
	//auto depthStencilViewObj = depthStencilView();
	//commandList_->OMSetRenderTargets(1, &currentBackBufferViewObj, true, &depthStencilViewObj);

	//// transition resource state
	//commandList_->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
	//	currentBackBuffer(),
	//	D3D12_RESOURCE_STATE_RENDER_TARGET,
	//	D3D12_RESOURCE_STATE_PRESENT
	//)));

	//// close command list
	//ThrowIfFailed(commandList_->Close());

	//// the commands to be executed added to command list
	//ID3D12CommandList *cmdLists[] = { commandList_.Get() };
	//commandQueue_->ExecuteCommandLists(static_cast<UINT>(std::size(cmdLists)), cmdLists);

	//// swap buffers
	//ThrowIfFailed(swapChain_->Present(0, 0));
	//currBackBuffer_ = (currBackBuffer_ + 1) % kSwapChainCount; 

	//// wait GPU render
	//flushCommandQueue();

		// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(directCmdListAlloc_->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(commandList_->Reset(directCmdListAlloc_.Get(), nullptr));

	// Indicate a state transition on the resource usage.
	commandList_->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)));

	// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
	commandList_->RSSetViewports(1, &screenViewport_);
	commandList_->RSSetScissorRects(1, &scissiorRect_);

	// Clear the back buffer and depth buffer.
	commandList_->ClearRenderTargetView(currentBackBufferView(), DX::Colors::LightSteelBlue, 0, nullptr);
	commandList_->ClearDepthStencilView(depthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	commandList_->OMSetRenderTargets(1, RVPtr(currentBackBufferView()), true, RVPtr(depthStencilView()));

	// Indicate a state transition on the resource usage.
	commandList_->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));

	// Done recording commands.
	ThrowIfFailed(commandList_->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(swapChain_->Present(0, 0));
	currBackBuffer_ = (currBackBuffer_ + 1) % kSwapChainCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	flushCommandQueue();
}

void TestGraphics::onResize() {
	Base::onResize();
}

void TestGraphics::handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	Base::handleMsg(hwnd, msg, wParam, lParam);
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

	const auto slotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
	const int instanced = 0;
	inputLayout_ = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(BoxVertex, position), slotClass, instanced },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(BoxVertex, color), slotClass, instanced }
	};
}

void TestGraphics::buildBoxGeometry() {
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
	memset(&psoDesc, 0, sizeof(psoDesc));
	psoDesc.pRootSignature = rootSignature_.Get();
	psoDesc.VS = { vsByteCode_->GetBufferPointer(), vsByteCode_->GetBufferSize() };
	psoDesc.PS = { psByteCode_->GetBufferPointer(), psByteCode_->GetBufferSize() };
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.InputLayout = { inputLayout_.data(), (UINT)inputLayout_.size() };
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = backBufferFormat_;
	psoDesc.DSVFormat = depthStencilFormat_;
	psoDesc.SampleDesc = { getSampleCount(), getSampleQuality() };

	ThrowIfFailed(d3dDevice_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso_)));
}
