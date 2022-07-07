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
#include <Script/LuaConfigLoader/LuaConfigLoader.h>

#include "Shader/D3DShaderResource.h"

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
	std::map<std::string, std::string> macros;
	for (size_t i = 0; i < size; ++i) {
#if defined(_DEBUG) || defined(DEBUG)
		auto iter = macros.find(pMacros[i].Definition);
		assert(iter == macros.end());
#endif
		if (pMacros[i].Name != nullptr)
			macros[pMacros[i].Definition] = pMacros[i].Name;
		else
			macros[pMacros[i].Definition] = std::string();
	}

	std::string result = name;
	for (auto &&[key, value] : macros) {
		if (value.empty())
			result += std::format("_[{}]", key);
		else
			result += std::format("_[{},{}]", key, value);
	}
	return result;
}


struct SplitMacroKeyHelper {
	bool handleName(const std::string &key) {
		if (index == key.length()) {
			name = key;
			return false;
		}

		if (key[index] == '_') {
			name = key.substr(0, index);
			pTickFun = &SplitMacroKeyHelper::findMacroBegin;
			position = index + 1;
		}
		return true;
	}

	// ´Ó _ ¿ªÕÒµ½ [
	bool findMacroBegin(const std::string &key) {
		if (index == key.length())
			return false;
		
		if (key[index] == '[') {
			position = index + 1;
			pTickFun = &SplitMacroKeyHelper::handleMacroKey;
		}
		assert(key[index] == '_' || key[index] == '[');
		return true;
	}

	bool handleMacroKey(const std::string &key) {
		assert(index < key.length());
		if (key[index] == ',') {
			macros.push_back(key.substr(position, index - position));
			position = index + 1;
			pTickFun = &SplitMacroKeyHelper::handleMacroValue;
		}
		if (key[index] == ']') {
			macros.push_back(key.substr(position, index - position));
			macros.push_back("");
			position = index + 1;
			pTickFun = &SplitMacroKeyHelper::findMacroBegin;
		}
		return true;
	}

	bool handleMacroValue(const std::string &key) {
		if (index == key.length()) {
			assert(false && "The macro key is not complete");
			return false;
		}

		if (key[index] == ']') {
			macros.push_back(key.substr(position, index-position));
			position = index + 1;
			pTickFun = &SplitMacroKeyHelper::findMacroBegin;
		}
		return true;
	}

	void parse(const std::string &key) {
		while ((this->*pTickFun)(key))
			++index;
	}

public:
	bool (SplitMacroKeyHelper:: *pTickFun)(const std::string &key) = &SplitMacroKeyHelper::handleName;
	size_t position = 0;
	size_t index = 0;
	std::string name;
	std::vector<std::string> macros;
};

void splitMacroKey(const std::string &key, std::string &name, std::vector<MacroPair> &macros) {
	if (key.empty())
		return;


	SplitMacroKeyHelper helper;
	helper.parse(key);
	name = std::move(helper.name);

	for (size_t i = 0; i < helper.macros.size(); i += 2) {
		MacroPair pair = {
			std::move(helper.macros[i+0]),
			std::move(helper.macros[i+1]),
		};
		macros.push_back(std::move(pair));
	}
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

	loadShaderDefineConfig();

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

void D3DInitializer::loadShaderDefineConfig() {
	auto file = getD3DResource("config/ShaderDefine.lua");
	scr::LuaConfigLoader luaConfigLoader(file.begin(), file.size());

	if (luaConfigLoader.beginTable("shaderList")) {
		for (luaConfigLoader.beginNext(); luaConfigLoader.next();) {
			auto key = luaConfigLoader.getKey();
			//assert(luaConfigLoader.getValueType() == scr::LuaValueType::Table);
			bool b = luaConfigLoader.isTable("file");
			if (luaConfigLoader.beginTable()) {
				std::string file = *luaConfigLoader.getString("file");
				if (auto vs = luaConfigLoader.getString("vs"))
					VSShaderManager::instance()->initShaderCreator(file, *vs, "VS_5_0");
				if (auto ps = luaConfigLoader.getString("ps"))
					PSShaderManager::instance()->initShaderCreator(file, *ps, "PS_5_0");
				if (auto gs = luaConfigLoader.getString("gs"))
					GSShaderManager::instance()->initShaderCreator(file, *gs, "GS_5_0");
				if (auto hs = luaConfigLoader.getString("hs"))
					GSShaderManager::instance()->initShaderCreator(file, *hs, "HS_5_0");
				if (auto ds = luaConfigLoader.getString("ds"))
					GSShaderManager::instance()->initShaderCreator(file, *ds, "DS_5_0");
				if (auto cs = luaConfigLoader.getString("cs"))
					GSShaderManager::instance()->initShaderCreator(file, *cs, "CS_5_0");
			}
			luaConfigLoader.endTable();
		}
	}
	luaConfigLoader.endTable();
}

}

static std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> converter;
std::string std::to_string(const std::wstring &wstr) {
	return converter.to_bytes(wstr);
}

std::wstring std::to_wstring(const std::string &str) {
	return converter.from_bytes(str);
}
