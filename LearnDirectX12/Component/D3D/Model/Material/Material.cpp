#include "Material.h"
#include <RenderGraph/Technique/Technique.h>

namespace d3d {

Material::Material(const std::string &name, d3d::INode *pNode, d3d::RenderItem *pRenderItem) : _materialName(name) {
	// todo: 在派生类中初始化 _techniques
}

const std::string & Material::getMaterialName() const {
	return _materialName;
}

size_t Material::getNumTechnique() const {
	return _techniques.size();
}

std::shared_ptr<rg::Technique> Material::getTechnique(size_t idx) const {
	if (idx >= _techniques.size()) {
		assert(false);
		return nullptr;
	}
	return _techniques[idx];
}

Material::~Material() = default;

}
