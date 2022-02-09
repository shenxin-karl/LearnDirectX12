#include "ShaderObject.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace dx12lib {

bool ShaderObject::loadShader() {
	if (_pShaderCode != nullptr)
		return;

	std::fstream fin(_shaderFile, std::ios::in);
	if (!fin.is_open()) {
		assert(false);
		std::cerr << "can't open the file: " << _shaderFile << std::endl;
		return;
	}

	std::stringstream sbuf;
	sbuf << fin.rdbuf();
	std::string content = sbuf.str();
	ThrowIfFailed(D3DCreateBlob(content.length(), &_pShaderCode));
	std::memcpy(_pShaderCode->GetBufferPointer(), content.data(), content.size());
}

ShaderObject::ShaderObject(const std::string &shaderFile) : _shaderFile(shaderFile) {
}

ShaderObject::ShaderObject(const std::string &shaderFile, WRL::ComPtr<ID3DBlob> pShaderCode)
: _shaderFile(shaderFile), _pShaderCode(pShaderCode) 
{
}

void ShaderObject::compileShader(WRL::ComPtr<ID3DBlob> &pByteCode, 
	const std::string &entrypoint, 
	const std::string &target,
	const D3D_SHADER_MACRO *defines)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	WRL::ComPtr<ID3DBlob> errors;
	HRESULT hr = D3DCompile(
		_pShaderCode->GetBufferPointer(),
		_pShaderCode->GetBufferSize(),
		nullptr,
		defines,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(),
		target.c_str(),
		compileFlags,
		0,
		&pByteCode,
		&errors
	);

	if (FAILED(hr)) {
		OutputDebugStringA(static_cast<char *>(errors->GetBufferPointer()));
		ThrowIfFailed(hr);
	}
}

}