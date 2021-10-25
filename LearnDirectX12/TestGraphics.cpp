#include <DirectXMath.h>
#include <DirectXColors.h>
#include <string_view>
#include "TestGraphics.h"
#include "D3DApp.h"
#include "window.h"

void TestGraphics::initialize() {
	Base::initialize();
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
	ThrowIfFailed(directCmdListAlloc_->Reset());
	ThrowIfFailed(commandList_->Reset(
		directCmdListAlloc_.Get(), nullptr
	));

	auto transitionDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		currentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	commandList_->ResourceBarrier(1, &transitionDesc); 

	commandList_->RSSetViewports(1, &screenViewport_);
	commandList_->RSSetScissorRects(1, &scissiorRect_);

	// clear back target view and depth and stencil buffer
	commandList_->ClearRenderTargetView(
		currentBackBufferView(),
		DirectX::Colors::LightSteelBlue,
		0,
		nullptr
	);
	commandList_->ClearDepthStencilView(
		depthStencilView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f, 0, 0, nullptr
	);

	auto currentBackBufferViewObj = currentBackBufferView();
	auto depthStencilViewObj = depthStencilView();
	commandList_->OMSetRenderTargets(1, &currentBackBufferViewObj, true, &depthStencilViewObj);

	// transition resource state
	transitionDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		currentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	commandList_->ResourceBarrier(1, &transitionDesc);

	// close command list
	ThrowIfFailed(commandList_->Close());

	// the commands to be executed added to command list
	ID3D12CommandList *cmdLists[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(static_cast<UINT>(std::size(cmdLists)), cmdLists);

	// swap buffers
	ThrowIfFailed(swapChain_->Present(0, 0));
	currBackBuffer_ = (currBackBuffer_ + 1) % kSwapChainCount; 

	// wait GPU render
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
	size_t objCBByteSize = calcConstantBufferByteSize(sizeof(BoxObjectConstant));
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
	assert(vsByteCode_ == nullptr);
	assert(psByteCode_ == nullptr);

	const auto slotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
	const int instanced = 0;
	inputLayout_ = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(BoxVertex, position), slotClass, instanced },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(BoxVertex, color), slotClass, instanced }
	};
}