#pragma once
#include <unordered_map>
#include <memory>
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Pipeline/PipelineStateObject.h>
#include <Singleton/Singleton.hpp>

namespace d3d {

template<typename T>
class PSOManager;

using GraphicsPSOManager = PSOManager<dx12lib::GraphicsPSO>;
using ComputePSOManager = PSOManager<dx12lib::ComputePSO>;

template<typename T>
class PSOManager : public com::Singleton<PSOManager<T>> {
public:
	std::shared_ptr<T> get(const std::string &key);
	void set(const std::string &key, std::shared_ptr<dx12lib::PSO> pPSO);
	void erase(const std::string &key);
	bool exist(const std::string &key);
private:
	using PSOCreator = std::function<std::shared_ptr<T>(const std::vector<MacroPair> &)>;
	std::unordered_map<std::string, PSOCreator> _psoCreatorMap;
	std::unordered_map<std::string, std::shared_ptr<dx12lib::PSO>> _psoMap;
};

template<typename T>
inline std::shared_ptr<T> PSOManager<T>::get(const std::string &key) {
	auto iter = _psoMap.find(key);
	if (iter != _psoMap.end())
		return std::static_pointer_cast<T>(iter->second);

	std::string name;
	std::vector<MacroPair> macros;
	splitMacroKey(key, name, macros);
	if (auto iter = _psoCreatorMap.find(name); iter != _psoCreatorMap.end()) {
		std::shared_ptr<T> pso = iter->second(macros);
		if (pso != nullptr) {
			_psoMap[key] = pso;
			return pso;
		}
	}
	return nullptr;
}

template<typename T>
inline void PSOManager<T>::set(const std::string &key, std::shared_ptr<dx12lib::PSO> pPSO) {
	assert(pPSO != nullptr);
	_psoMap[key] = pPSO;
}

template<typename T>
inline void PSOManager<T>::erase(const std::string &key) {
	_psoMap.erase(key);
}

template<typename T>
inline bool PSOManager<T>::exist(const std::string &key) {
	auto iter = _psoMap.find(key);
	return iter != _psoMap.end();
}

}
