#define _CRT_SECURE_NO_WARNINGS
#define  NOMINMAX
#include <windows.h>
#include <cassert>
#include <d3dcompiler.h>
#include "d3dutil.h"
#include "D3D/Tool/D3Dx12.h"
#include "D3D/Exception/D3DException.h"
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

}

static std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> converter;
std::string std::to_string(const std::wstring &wstr) {
	return converter.to_bytes(wstr);
}

std::wstring std::to_wstring(const std::string &str) {
	return converter.from_bytes(str);
}
