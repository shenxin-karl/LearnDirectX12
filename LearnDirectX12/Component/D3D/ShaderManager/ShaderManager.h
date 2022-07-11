#pragma once
#include <unordered_map>
#include <Singleton/Singleton.hpp>
#include <d3d12.h>
#include <functional>
#include <D3D/d3dutil.h>

#include "D3D/Shader/D3DShaderResource.h"

CMRC_DECLARE(D3D);


namespace d3d {

enum class ShaderType {
	INVALID = -1,
	VS = 0,
	PS = 1,
	GS = 2,
	HS = 3,
	DS = 4,
	CS = 5,
};

template<ShaderType>
class ShaderManager;

using VSShaderManager = ShaderManager<ShaderType::VS>;
using PSShaderManager = ShaderManager<ShaderType::PS>;
using GSShaderManager = ShaderManager<ShaderType::GS>;
using HSShaderManager = ShaderManager<ShaderType::HS>;
using DSShaderManager = ShaderManager<ShaderType::DS>;
using CDShaderManager = ShaderManager<ShaderType::CS>;


class Shader {
	template<ShaderType>
	friend class ShaderManager;
public:
	Shader() = default;
	Shader(ShaderType type, WRL::ComPtr<ID3DBlob> pByteCode);
	ShaderType getShaderType() const;
	WRL::ComPtr<ID3DBlob> getByteCode() const;
	operator WRL::ComPtr<ID3DBlob>() const;
	operator bool() const;
private:
	ShaderType _type = ShaderType::INVALID;
	WRL::ComPtr<ID3DBlob> _pByteCode;
};

template<ShaderType Type> 
class ShaderManager : public com::Singleton<ShaderManager<Type>> {
public:
	Shader get(const std::string &key) const;
	void set(const std::string &key, WRL::ComPtr<ID3DBlob> pByteCode);
	bool exist(const std::string &key) const;
	void erase(const std::string &key);
	void initShaderCreator(const std::string &path, const std::string &entryPoint, const std::string &target);
private:
	using ShaderCreator = std::function<Shader(const std::vector<MacroPair> &)>;
	std::unordered_map<std::string, Shader> _shaders;
	std::unordered_map<std::string, ShaderCreator> _shaderCreatorMap;
};

template<ShaderType Type>
inline Shader ShaderManager<Type>::get(const std::string &key) const {
	auto iter = _shaders.find(key);
	if (iter != _shaders.end())
		return iter->second;
	return Shader{};
}

template<ShaderType Type>
inline void ShaderManager<Type>::set(const std::string &key, WRL::ComPtr<ID3DBlob> pByteCode) {
	assert(!key.empty());
	assert(pByteCode != nullptr);
	_shaders[key] = Shader(Type, pByteCode);
}

template<ShaderType Type>
inline bool ShaderManager<Type>::exist(const std::string &key) const {
	auto iter = _shaders.find(key);
	return iter != _shaders.end();
}

template<ShaderType Type>
inline void ShaderManager<Type>::erase(const std::string &key) {
	_shaders.erase(key);
}

template <ShaderType Type>
void ShaderManager<Type>::initShaderCreator(const std::string &path, const std::string &entryPoint, const std::string &target) {
	std::string name;
	size_t end = path.find_last_not_of(".");
	size_t begin = path.find_last_of("/");
	if (begin == std::string::npos)
		begin = path.find_last_of("\\");

	begin += 1;		// Ìø¹ý /
	if (begin >= end || begin == std::string::npos || end == std::string::npos) {
		assert(false && "Invalid shader path name");
		return;
	}
	
	std::string fileName = path.substr(begin, end - begin);
	if (auto iter = _shaderCreatorMap.find(fileName); iter != _shaderCreatorMap.end()) {
		assert(false && "Duplicate shader Creator");
		return;
	}

	_shaderCreatorMap[fileName] = [=](const std::vector<MacroPair> &macros) {
		std::vector<D3D_SHADER_MACRO> defines(macros.size() + 1);
		defines.back().Definition = nullptr;
		defines.back().Name = nullptr;
		for (size_t i = 0; i < macros.size(); ++i) {
			defines[i].Definition = macros[i].key.c_str();
			defines[i].Name = (macros[i].value.empty()) ? nullptr : macros[i].value.c_str();
		}
	
		auto fs = cmrc::D3D::get_filesystem();
		if (!fs.exists(path)) {
			auto file = fs.open(path);
			auto pByteCode = d3d::compileShader(file.begin(), 
				file.size(), 
				defines.data(), 
				entryPoint, 
				target
			);
			return Shader(Type, pByteCode);
		}

		auto pByteCode = d3d::compileShader(std::to_wstring(path), defines.data(), entryPoint, target);
		return Shader(Type, pByteCode);
	};
}

}
