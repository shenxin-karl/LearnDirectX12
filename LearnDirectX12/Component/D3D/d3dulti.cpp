#include <windows.h>
#include <cassert>
#include <d3dcompiler.h>
#include "d3dulti.h"
#include "D3DX12.h"
#include "D3DException.h"

namespace WRL = Microsoft::WRL;
void _ThrowIfFailedImpl(const char *file, int line, HRESULT hr) {
	if (FAILED(hr))
		throw d3d::D3DException(hr, file, line);
	return;
}

WRL::ComPtr<ID3D12Resource> createDefaultBuffer(
	ID3D12Device *device, 
	ID3D12GraphicsCommandList *cmdList, 
	const void *initData, 
	UINT64 byteSize, 
	WRL::ComPtr<ID3D12Resource> &uploadBuffer
) {
	assert(device != nullptr && "createDefaultBuffer device is nullptr");
	assert(cmdList != nullptr && "createDefaultBuffer cmdList is nullptr");

	// create default heap
	WRL::ComPtr<ID3D12Resource> defaultBuffer;
	ThrowIfFailed(device->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(CD3DX12_RESOURCE_DESC::Buffer(byteSize)),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&defaultBuffer)
	));

	// create upload heap
	ThrowIfFailed(device->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(CD3DX12_RESOURCE_DESC::Buffer(byteSize)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadBuffer)
	));

	// describes the data we want to copy to the default buffer
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// copy the data to upload heap using the UpdateResources function
	cmdList->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
		defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST
	)));

	UpdateSubresources(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
		defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_GENERIC_READ
	)));
	return defaultBuffer;
}

size_t calcConstantBufferByteSize(size_t size) {
	constexpr size_t mask = ~0xff;
	return (size + 255) & mask;
}

Microsoft::WRL::ComPtr<ID3DBlob> compileShader(
	const std::wstring		&fileName, 
	const D3D_SHADER_MACRO	*defines, 
	const std::string		&entrypoint, 
	const std::string		&target) 
{
	UINT compilesFlags = 0;
#if defined(DEBUG) || defined(_DEBUG) 
	compilesFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	using Microsoft::WRL::ComPtr;
	HRESULT hr = S_OK;
	ComPtr<ID3DBlob> byteCode;
	ComPtr<ID3DBlob> errors;
	hr = D3DCompileFromFile(
		fileName.c_str(),
		defines,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(),
		target.c_str(),
		compilesFlags,
		0,
		&byteCode,
		&errors
	);

	if (FAILED(hr)) {
		OutputDebugStringA(static_cast<char *>(errors->GetBufferPointer()));
		ThrowIfFailed(hr);
	}
	return byteCode;
}

D3D12_VERTEX_BUFFER_VIEW MeshGeometry::getVertexBufferView() const {
	return {
		vertexBufferGPU->GetGPUVirtualAddress(),
		vertexBufferByteSize,
		vertexByteStride,
	};
}

D3D12_INDEX_BUFFER_VIEW MeshGeometry::getIndexBufferView() const {
	return {
		indexBufferGPU->GetGPUVirtualAddress(),
		indexBufferByteSize,
		indexBufferFormat,
	};
}

void MeshGeometry::disposeUploaders() {
	vertexBufferUploader = nullptr;
	indexBufferUploader = nullptr;
}
