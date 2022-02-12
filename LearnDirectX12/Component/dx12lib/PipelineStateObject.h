#pragma once
#include "dx12libStd.h"

namespace dx12lib {

class PSO {
public:
	explicit PSO(const std::string &name);
	PSO(const PSO &) = delete;
	PSO &operator=(const PSO &) = delete;
	void setRootSignature(std::shared_ptr<RootSignature> pRootSignature);
	std::shared_ptr<RootSignature> getRootSignature() const;
	WRL::ComPtr<ID3D12PipelineState> getPipelineStateObject() const;
	const std::string &getName() const;
	virtual void finalize(std::weak_ptr<Device> pDevice) = 0;
	virtual std::shared_ptr<PSO> clone(const std::string &name) = 0;
	virtual ~PSO() = default;
protected:
	std::string                      _name;
	std::shared_ptr<RootSignature>   _pRootSignature;
	WRL::ComPtr<ID3D12PipelineState> _pPSO;
};

class GraphicsPSO : public PSO {
public:
	explicit GraphicsPSO(const std::string &name);
	void setBlendState(const D3D12_BLEND_DESC &blendDesc);
	void setRasterizerState(const D3D12_RASTERIZER_DESC &rasterizerDesc);
	void setDepthStencilState(const D3D12_DEPTH_STENCIL_DESC &depthStencilDesc);
	void setSampleMask(UINT sampleMask);
	void setPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveType);
	void setDepthTargetFormat(DXGI_FORMAT DSVFormat, UINT msaaCount = 1, UINT massQuality = 1);
	void setRenderTargetFormat(DXGI_FORMAT RTVFormat, 
		DXGI_FORMAT DSVFormat, 
		UINT MsaaCount = 1, 
		UINT MsaaQuality = 1
	);
	void setRenderTargetFormats(UINT numRTVFormat, 
		const DXGI_FORMAT *pRTVFormat, 
		DXGI_FORMAT DSVFormat,
		UINT MsaaCount = 1, 
		UINT MsaaQuality = 0
	);
	void setInputLayout(const std::vector<D3D12_INPUT_ELEMENT_DESC> &inputLayout);
	void setPrimitiveRestart(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBProps);
	
	void setVertexShader(const void *pBinary, size_t size);
	void setPixelShader(const void *pBinary, size_t size);
	void setGeometryShader(const void *pBinary, size_t size);
	void setHullShader(const void *pBinary, size_t size);
	void setDomainShader(const void *pBinary, size_t size);

	void setVertexShader(WRL::ComPtr<ID3DBlob> pBytecode);
	void setPixelShader(WRL::ComPtr<ID3DBlob> pBytecode);
	void setGeometryShader(WRL::ComPtr<ID3DBlob> pBytecode);
	void setHullShader(WRL::ComPtr<ID3DBlob> pBytecode);
	void setDomainShader(WRL::ComPtr<ID3DBlob> pBytecode);

	void setVertexShader(const D3D12_SHADER_BYTECODE &bytecode);
	void setPixelShader(const D3D12_SHADER_BYTECODE &bytecode);
	void setGeometryShader(const D3D12_SHADER_BYTECODE &bytecode);
	void setHullShader(const D3D12_SHADER_BYTECODE &bytecode);
	void setDomainShader(const D3D12_SHADER_BYTECODE &bytecode);
	virtual void finalize(std::weak_ptr<Device> pDevice) override;
	virtual std::shared_ptr<PSO> clone(const std::string &name) override;
private:
	D3D12_SHADER_BYTECODE cacheBytecode(const std::string &name, const void *pData, size_t size);
	D3D12_SHADER_BYTECODE cacheBytecode(const std::string &name, WRL::ComPtr<ID3DBlob> pBytecode);
private:
	bool                                          _dirty = true;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC            _psoDesc;
	std::shared_ptr<D3D12_INPUT_ELEMENT_DESC[]>   _pInputLayout;
	std::map<std::string, WRL::ComPtr<ID3DBlob>>  _shaderBytecodeCache;
};


}