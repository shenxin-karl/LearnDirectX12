#include "ShaderManager.h"

namespace d3d {

Shader::Shader(ShaderType type, WRL::ComPtr<ID3DBlob> pByteCode) : _type(type), _pByteCode(pByteCode) {
}

ShaderType Shader::getShaderType() const {
	return _type;
}

WRL::ComPtr<ID3DBlob> Shader::getByteCode() const {
	return _pByteCode;
}

Shader::operator WRL::ComPtr<ID3DBlob>() const {
	return _pByteCode;
}

Shader::operator bool() const {
	return _type != ShaderType::INVALID && _pByteCode != nullptr;
}

}