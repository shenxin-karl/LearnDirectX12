#pragma once
#include <vector>
#include <memory>
#include <RenderGraph/Technique/Technique.h>
#include <D3D/d3dutil.h>

namespace d3d {

struct INode;
class RenderItem;
class Material : public NonCopyable {
public:
	Material(const std::string &name, d3d::INode *pNode, d3d::RenderItem *pRenderItem);
	const std::string &getMaterialName() const;
	size_t getNumTechnique() const;
	std::shared_ptr<rgph::Technique> getTechnique(size_t idx) const;
	~Material() override;
protected:
	std::string _materialName;
	std::vector<std::shared_ptr<rgph::Technique>> _techniques;
};

}
