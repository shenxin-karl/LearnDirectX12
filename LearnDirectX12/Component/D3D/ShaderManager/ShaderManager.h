#pragma once
#include <unordered_map>
#include <Singleton/Singleton.hpp>
#include <d3d12.h>
#include <D3D/d3dutil.h>

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
private:
	std::unordered_map<std::string, Shader> _shaders;
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
	_shaders[key] = pByteCode;
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

}