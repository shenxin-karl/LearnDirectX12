#include "PipelineStateObject.h"
#include "RootSignature.h"
#include "Device.h"
#include "MakeObejctTool.hpp"

namespace dx12lib {

PSO::PSO(std::weak_ptr<Device> pDevice, const std::string &name) : _name(name) {
}

void PSO::setRootSignature(std::shared_ptr<RootSignature> pRootSignature) {
	_pRootSignature = pRootSignature;
	_dirty = true;
}

std::shared_ptr<RootSignature> PSO::getRootSignature() const {
	return _pRootSignature;
}

WRL::ComPtr<ID3D12PipelineState> PSO::getPipelineStateObject() const {
	return _pPSO;
}

const std::string &PSO::getName() const {
	return _name;
}

GraphicsPSO::GraphicsPSO(std::weak_ptr<Device> pDevice, const std::string &name) : PSO(pDevice, name) {
	/// graphics pipeline static object has default state
	std::memset(&_psoDesc, 0, sizeof(_psoDesc));
	_psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	_psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	_psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	_psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	_psoDesc.SampleMask = 0xffffffff;
}

void GraphicsPSO::setBlendState(const D3D12_BLEND_DESC& blendDesc) {
	_psoDesc.BlendState = blendDesc;
	_dirty = true;
}

void GraphicsPSO::setRasterizerState(const D3D12_RASTERIZER_DESC& rasterizerDesc) {
	_psoDesc.RasterizerState = rasterizerDesc;
	_dirty = true;
}

void GraphicsPSO::setDepthStencilState(const D3D12_DEPTH_STENCIL_DESC &depthStencilDesc) {
	_psoDesc.DepthStencilState = depthStencilDesc;
	_dirty = true;
}

void GraphicsPSO::setSampleMask(UINT sampleMask) {
	_psoDesc.NodeMask = sampleMask;
	_dirty = true;
}

void GraphicsPSO::setPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveType) {
	_psoDesc.PrimitiveTopologyType = primitiveType;
	_dirty = true;
}

void GraphicsPSO::setDepthTargetFormat(DXGI_FORMAT DSVFormat) {
	setRenderTargetFormats(0, nullptr, DSVFormat);
}

void GraphicsPSO::setRenderTargetFormat(DXGI_FORMAT RTVFormat, DXGI_FORMAT DSVFormat) {
	setRenderTargetFormats(1, &RTVFormat, DSVFormat);
}

void GraphicsPSO::setRenderTargetFormats(UINT numRTVFormat, const DXGI_FORMAT *pRTVFormat, DXGI_FORMAT DSVFormat) 
{
	assert((numRTVFormat != 0 && pRTVFormat != nullptr) || (numRTVFormat == 0));
	for (UINT i = 0; i < numRTVFormat; ++i) {
		assert(pRTVFormat[i] != DXGI_FORMAT_UNKNOWN);
		_psoDesc.RTVFormats[i] 
			= pRTVFormat[i];
	}
	for (UINT i = numRTVFormat; i < _psoDesc.NumRenderTargets; ++i)
		_psoDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
	_psoDesc.NumRenderTargets = numRTVFormat;
	_psoDesc.DSVFormat = DSVFormat;
	_psoDesc.SampleDesc.Count = 1;
	_psoDesc.SampleDesc.Quality = 0;
	_dirty = true;
}

void GraphicsPSO::setInputLayout(const std::vector<D3D12_INPUT_ELEMENT_DESC> &inputLayout) {
	_dirty = true;
	if (inputLayout.empty()) {
		_psoDesc.InputLayout.NumElements = 0;
		_psoDesc.InputLayout.pInputElementDescs = nullptr;
		return;
	}

	_pInputLayout = std::shared_ptr<D3D12_INPUT_ELEMENT_DESC[]>(new D3D12_INPUT_ELEMENT_DESC[inputLayout.size()]);
	for (std::size_t i = 0; i < inputLayout.size(); ++i)
		_pInputLayout[i] = inputLayout[i];

	_psoDesc.InputLayout.NumElements = static_cast<UINT>(inputLayout.size());
	_psoDesc.InputLayout.pInputElementDescs = _pInputLayout ? _pInputLayout.get() : nullptr;
}

void GraphicsPSO::setPrimitiveRestart(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBProps) {
	_psoDesc.IBStripCutValue = IBProps;
	_dirty = true;
}
void GraphicsPSO::setVertexShader(const void *pBinary, size_t size) {
	_psoDesc.VS = cacheBytecode("VS", pBinary, size);
}
void GraphicsPSO::setPixelShader(const void *pBinary, size_t size) {
	_psoDesc.PS = cacheBytecode("PS", pBinary, size);
}
void GraphicsPSO::setGeometryShader(const void *pBinary, size_t size) {
	_psoDesc.GS = cacheBytecode("GS", pBinary, size);
}
void GraphicsPSO::setHullShader(const void *pBinary, size_t size) {
	_psoDesc.HS = cacheBytecode("HS", pBinary, size);
}
void GraphicsPSO::setDomainShader(const void *pBinary, size_t size) {
	_psoDesc.DS = cacheBytecode("DS", pBinary, size);
}

bool GraphicsPSO::isDirty() const {
	return _dirty;
}

void GraphicsPSO::setVertexShader(WRL::ComPtr<ID3DBlob> pBytecode) {
	_psoDesc.VS = cacheBytecode("VS", pBytecode);
}
void GraphicsPSO::setPixelShader(WRL::ComPtr<ID3DBlob> pBytecode) {
	_psoDesc.PS = cacheBytecode("PS", pBytecode);
}
void GraphicsPSO::setGeometryShader(WRL::ComPtr<ID3DBlob> pBytecode) {
	_psoDesc.GS = cacheBytecode("GS", pBytecode);
}
void GraphicsPSO::setHullShader(WRL::ComPtr<ID3DBlob> pBytecode) {
	_psoDesc.HS = cacheBytecode("HS", pBytecode);
}
void GraphicsPSO::setDomainShader(WRL::ComPtr<ID3DBlob> pBytecode) {
	_psoDesc.DS = cacheBytecode("DS", pBytecode);
}

void GraphicsPSO::setVertexShader(const D3D12_SHADER_BYTECODE &bytecode) {
	setVertexShader(bytecode.pShaderBytecode, bytecode.BytecodeLength);
}
void GraphicsPSO::setPixelShader(const D3D12_SHADER_BYTECODE &bytecode) {
	setPixelShader(bytecode.pShaderBytecode, bytecode.BytecodeLength);
}
void GraphicsPSO::setGeometryShader(const D3D12_SHADER_BYTECODE &bytecode) {
	setGeometryShader(bytecode.pShaderBytecode, bytecode.BytecodeLength);
}
void GraphicsPSO::setHullShader(const D3D12_SHADER_BYTECODE &bytecode) {
	setHullShader(bytecode.pShaderBytecode, bytecode.BytecodeLength);
}
void GraphicsPSO::setDomainShader(const D3D12_SHADER_BYTECODE &bytecode) {
	setDomainShader(bytecode.pShaderBytecode, bytecode.BytecodeLength);
}

void GraphicsPSO::finalize() {
	if (!_dirty)
		return;
	
	assert(_pRootSignature != nullptr);
	_psoDesc.pRootSignature = _pRootSignature->getRootSignature().Get();
	ThrowIfFailed(_pDevice.lock()->getD3DDevice()->CreateGraphicsPipelineState(
		&_psoDesc,
		IID_PPV_ARGS(&_pPSO)
	));
	_dirty = false;
}

std::shared_ptr<PSO> GraphicsPSO::clone(const std::string &name) {
	auto pRes = std::make_shared<MakeGraphicsPSO>(_pDevice, name);
	pRes->_dirty = this->_dirty;
	pRes->_pPSO = this->_pPSO;
	pRes->_psoDesc = this->_psoDesc;
	pRes->_pRootSignature = this->_pRootSignature;
	pRes->_shaderBytecodeCache = this->_shaderBytecodeCache;
	return std::static_pointer_cast<PSO>(pRes);
}

D3D12_SHADER_BYTECODE GraphicsPSO::cacheBytecode(const std::string &name, const void *pData, size_t size) {
	if (pData == nullptr) {
		_shaderBytecodeCache.erase(name);
		return { nullptr, 0 };
	}
	_dirty = true;
	WRL::ComPtr<ID3DBlob> pBuffer;
	ThrowIfFailed(D3DCreateBlob(size, &pBuffer));
	std::memcpy(pBuffer->GetBufferPointer(), pData, size);
	_shaderBytecodeCache[name] = pBuffer;
	return { pBuffer->GetBufferPointer(), pBuffer->GetBufferSize() };
}

D3D12_SHADER_BYTECODE GraphicsPSO::cacheBytecode(const std::string &name, WRL::ComPtr<ID3DBlob> pBytecode) {
	if (pBytecode == nullptr) {
		_shaderBytecodeCache.erase(name);
		return { nullptr, 0 };
	}
	_dirty = true;
	_shaderBytecodeCache[name] = pBytecode;
	return { pBytecode->GetBufferPointer(), pBytecode->GetBufferSize() };
}

}
