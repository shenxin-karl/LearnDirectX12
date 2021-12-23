#pragma once
#include <winnt.h>
#include <d3d12.h>
#include <wrl.h>
#include <utility>
#include <string>
#include <DirectXMath.h>
#include <directxcollision.h>
#include <unordered_map>
#include "D3Dx12.h"

void _ThrowIfFailedImpl(const char *file, int line, HRESULT hr);
#define ThrowIfFailed(hr) (_ThrowIfFailedImpl(__FILE__, __LINE__, hr))

Microsoft::WRL::ComPtr<ID3D12Resource> createDefaultBuffer(
	ID3D12Device *device,
	ID3D12GraphicsCommandList *cmdList,
	const void *initData,
	UINT64 byteSize,
	Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer
);

size_t calcConstantBufferByteSize(size_t size);


Microsoft::WRL::ComPtr<ID3DBlob> compileShader(
	const std::wstring		&fileName,
	const D3D_SHADER_MACRO	*defines,
	const std::string		&entrypoint,
	const std::string		&target
);


#define USE_RVPTR_FUNC_IMPL
#ifdef USE_RVPTR_FUNC_IMPL
	template<typename T>
	const T *_rightValuePtr(const T &val) {
		return &val;
	}
// use thread local static object save return value. return value ptr
#define RVPtr(val) _rightValuePtr(val)
#else
#define RVPtr(val) (&val)
#endif


struct SubmeshGeometry {
	UINT	indexCount = 0;
	UINT	startIndexLocation = 0;
	INT		baseVertexLocation = 0;
	DirectX::BoundingBox bounds;
};

struct MeshGeometry {
	D3D12_VERTEX_BUFFER_VIEW getVertexBufferView() const;
	D3D12_INDEX_BUFFER_VIEW getIndexBufferView() const;
	void disposeUploaders();
public:
	std::string								name;
	Microsoft::WRL::ComPtr<ID3DBlob>		vertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob>		indexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>	vertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>	indexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>	vertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>	indexBufferUploader = nullptr;

	UINT vertexByteStride = 0;
	UINT vertexBufferByteSize = 0;
	DXGI_FORMAT indexBufferFormat = DXGI_FORMAT_R16_UINT;
	UINT indexBufferByteSize = 0;

	std::unordered_map<std::string, SubmeshGeometry> drawArgs;
};

namespace d3dUtil {

const CD3DX12_STATIC_SAMPLER_DESC &getStaticSamplerPointWrap();
const CD3DX12_STATIC_SAMPLER_DESC &getStaticSamplerPointClamp();
const CD3DX12_STATIC_SAMPLER_DESC &getStaticSamplerLinearWrap();
const CD3DX12_STATIC_SAMPLER_DESC &getStaticSamplerLinearClamp();
const CD3DX12_STATIC_SAMPLER_DESC &getStaticSamplerAnisotropicWrap();
const CD3DX12_STATIC_SAMPLER_DESC &getStaticSamplerAnisotropicClamp();
const std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> &getStaticSamplers();

}