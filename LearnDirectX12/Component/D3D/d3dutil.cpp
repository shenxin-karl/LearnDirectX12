#define _CRT_SECURE_NO_WARNINGS
#define  NOMINMAX
#include <windows.h>
#include <cassert>
#include <d3dcompiler.h>
#include <format>
#include "d3dutil.h"
#include "D3D/Tool/D3Dx12.h"
#include "D3D/Exception/D3DException.h"
#include "D3D/TextureManager/TextureManager.h"
#include "D3D/PSOManager/PSOManager.hpp"
#include "D3D/ShaderManager/ShaderManager.h"
#define SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <locale>

#define STB_INCLUDE_IMPLEMENTATION
#include <iostream>
#include <stb/stb_include.h>

namespace d3d {

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
		OutputDebugString(static_cast<char *>(errors->GetBufferPointer()));
		ThrowIfFailed(hr);
	}
	return byteCode;
}


WRL::ComPtr<ID3DBlob> compileShader(const char *fileContext, 
	std::size_t sizeInByte,
	const D3D_SHADER_MACRO *defines, 
	const std::string &entrypoint, 
	const std::string &target) 
{
	UINT compilesFlags = 0;
#if defined(DEBUG) || defined(_DEBUG) 
	compilesFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	HRESULT hr = S_OK;
	WRL::ComPtr<ID3DBlob> byteCode;
	WRL::ComPtr<ID3DBlob> errors;
	hr = D3DCompile(fileContext,
		sizeInByte,
		nullptr,
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
		OutputDebugString(static_cast<char *>(errors->GetBufferPointer()));
		ThrowIfFailed(hr);
	}
	return byteCode;
}

WRL::ComPtr<ID3DBlob> compileShaderParseInclude(const char *pFileName, const char *fileContext, std::size_t sizeInByte,
	const D3D_SHADER_MACRO *defines, const std::string &entrypoint, const std::string &target)
{
	std::string context(fileContext, sizeInByte);
	std::string fileName = std::string("\"") + pFileName + "\"";
	char error[256];
	fileContext = stb_include_string(
		const_cast<char *>(context.c_str()), 
		nullptr, 
		const_cast<char *>(D3D_HLSL_SHADER_PATH), 
		const_cast<char *>(fileName.c_str()),
		error
	);
	if (fileContext == nullptr) {
		std::cerr << error << std::endl;
		return nullptr;
	}

	size_t stringLength = strlen(fileContext);
	auto pByteCode = compileShader(fileContext, stringLength, defines, entrypoint, target);
	return pByteCode;
}

std::string calcMacroKey(const std::string &name, const std::vector<D3D_SHADER_MACRO> &macros) {
	return calcMacroKey(name, macros.data(), macros.size());
}

std::string calcMacroKey(const std::string &name, const D3D_SHADER_MACRO *pMacros, size_t size) {
	std::vector<std::string> keys;
	keys.reserve(size);
	for (size_t i = 0; i < size; ++i) {
		D3D_SHADER_MACRO shaderMacro = pMacros[i];
		if (shaderMacro.Definition == nullptr)
			continue;

		if (shaderMacro.Definition != nullptr && shaderMacro.Name != nullptr)
			keys.push_back(std::format("_{}#{}", shaderMacro.Definition, shaderMacro.Name));
		else
			keys.push_back(std::format("_{}", shaderMacro.Definition));
	}

	std::string result = name;
	std::stable_sort(keys.begin(), keys.end());
	for (auto &key : keys)
		result += key;
	return result;
}

D3DInitializer::D3DInitializer() {
	bool testFlag = false;
	if (!isInited.compare_exchange_strong(testFlag, true)) {
		assert(false && "Initialize D3D repeatedly");
		std::abort();
	}
	
	VSShaderManager::emplace();
	PSShaderManager::emplace();
	GSShaderManager::emplace();
	HSShaderManager::emplace();
	DSShaderManager::emplace();
	CDShaderManager::emplace();
	GraphicsPSOManager::emplace();
	ComputePSOManager::emplace();
	TextureManager::emplace();
}

D3DInitializer::~D3DInitializer() {
	bool testFlag = true;
	if (!isInited.compare_exchange_strong(testFlag, true)) {
		assert(false && "destroy D3D repeatedly");
		std::abort();
	}

	TextureManager::destroy();
	ComputePSOManager::destroy();
	GraphicsPSOManager::destroy();
	CDShaderManager::destroy();
	DSShaderManager::destroy();
	HSShaderManager::destroy();
	GSShaderManager::destroy();
	PSShaderManager::destroy();
	VSShaderManager::destroy();
}

}

static std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> converter;
std::string std::to_string(const std::wstring &wstr) {
	return converter.to_bytes(wstr);
}

std::wstring std::to_wstring(const std::string &str) {
	return converter.from_bytes(str);
}
