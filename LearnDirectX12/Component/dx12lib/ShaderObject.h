#pragma once
#include "dx12libStd.h"

namespace dx12lib {

class ShaderObject {
public:
	ShaderObject(const std::wstring &shaderFile);
	ShaderObject(const std::wstring &shaderFile, WRL::ComPtr<ID3DBlob> pShaderCode);
	ShaderObject(const ShaderObject &) = delete;
protected:
	void compileShader(WRL::ComPtr<ID3DBlob> &pByteCode, 
		const D3D_SHADER_MACRO *defines, 
		const std::string &entrypoint,
		const std::string &target
	);
protected:
	std::wstring _shaderFile;
	WRL::ComPtr<ID3DBlob> _pShaderCode;
	WRL::ComPtr<ID3DBlob> _pVSByteCode;
	WRL::ComPtr<ID3DBlob> _pFSByteCode;
	WRL::ComPtr<ID3DBlob> _pHSByteCode;
	WRL::ComPtr<ID3DBlob> _pDSByteCode;
	WRL::ComPtr<ID3DBlob> _pGSByteCode;
	WRL::ComPtr<ID3DBlob> _pCSByteCode;
	std::shared_ptr<RootSignature> _pRootSignature;
	std::shared_ptr<PipelineStateObject> _pPSO;
};

}