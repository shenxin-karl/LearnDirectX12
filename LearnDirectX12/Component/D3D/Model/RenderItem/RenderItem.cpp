#include "RenderItem.h"
#include "D3D/Model/Material/Material.h"

namespace d3d {

RenderItem::RenderItem(std::shared_ptr<ALMesh> pALMesh) : _pMesh(pALMesh) {
}

std::shared_ptr<Material> RenderItem::getMaterial() const {
	return _pMaterial;
}

std::shared_ptr<ALMesh> RenderItem::getMesh() const {
	return _pMesh;
}

void RenderItem::setMaterial(std::shared_ptr<Material> pMaterial) {
	_pMaterial = pMaterial;
}

void RenderItem::rebuildTechnique() {
	clearTechnique();
	if (_pMaterial == nullptr)
		return;

	for (size_t i = 0; i < _pMaterial->getNumTechnique(); ++i)
		addTechnique(_pMaterial->getTechnique(i));
}

}
