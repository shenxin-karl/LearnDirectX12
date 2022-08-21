#pragma once
#include <RenderGraph/Drawable/Drawable.h>
#include "D3D/AssimpLoader/ALNode.h"

namespace d3d {

struct INode;
class Material;
class RenderItem : public rg::Drawable {
public:
	RenderItem(std::shared_ptr<ALMesh> pALMesh);
	std::shared_ptr<Material> getMaterial() const;
	std::shared_ptr<ALMesh> getMesh() const;
	void setMaterial(std::shared_ptr<Material> pMaterial);
	void rebuildTechnique();
private:
	std::shared_ptr<ALMesh> _pMesh;
	std::shared_ptr<Material> _pMaterial;
};

}
