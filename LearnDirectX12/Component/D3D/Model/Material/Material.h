#pragma once
#include <string>

namespace d3d {

#define DECLARE_MATERIAL_FUNC(name)			\
	const auto &get##name() const {			\
		return _##name;						\
	}										\
	void set##name(const auto &value) {		\
		_##name = value;					\
	}

class Material {
public:
	DECLARE_MATERIAL_FUNC(albedoMapName)
	DECLARE_MATERIAL_FUNC(aoMapName)
	DECLARE_MATERIAL_FUNC(normalMapName)
	DECLARE_MATERIAL_FUNC(metallicMapName)
	DECLARE_MATERIAL_FUNC(roughnessMapName)
	DECLARE_MATERIAL_FUNC(castShadow)
private:
	std::string _albedoMapName;
	std::string _aoMapName;
	std::string _normalMapName;
	std::string _metallicMapName;
	std::string _roughnessMapName;
private:
	bool _castShadow : 1 = false;
};	

#undef DECLARE_MATERIAL_FUNC

}
