#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <utility>
#include <string>
#include <DirectXMath.h>
#include <directxcollision.h>
#include <unordered_map>
#include <array>
#include "d3dx12.h"
#include "ComponentStd.h"


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

namespace WRL = Microsoft::WRL;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using ::ThrowIfFailed;

const CD3DX12_STATIC_SAMPLER_DESC &getPointWrapStaticSampler();
const CD3DX12_STATIC_SAMPLER_DESC &getPointClampStaticSampler();
const CD3DX12_STATIC_SAMPLER_DESC &getLinearWrapStaticSampler();
const CD3DX12_STATIC_SAMPLER_DESC &getLinearClampStaticSampler();
const CD3DX12_STATIC_SAMPLER_DESC &getAnisotropicWrapStaticSampler();
const CD3DX12_STATIC_SAMPLER_DESC &getAnisotropicClampStaticSampler();
const std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> &getStaticSamplers();

class CameraBase;
class CoronaCamera;

}