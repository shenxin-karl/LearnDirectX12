#pragma once
#include <memory>
#include <string>
#include <vector>
#include <RenderGraph/RenderGraphStd.h>
#include <RenderGraph/Technique/TechniqueType.hpp>
#include <RenderGraph/Technique/Step.h>

namespace rg {

class Technique {
public:
	Technique(const std::string &techniqueName, TechniqueType type, bool active = true);
	Technique(const Technique &) = delete;
	void addStep(std::unique_ptr<Step> pStep);
	void submit(const Drawable &drawable, const TechniqueFlag &techniqueFlag) const;
	void setActive(bool bActive);
	bool isActive() const;
	TechniqueType getTechniqueType() const;
	const std::string &getTechniqueName() const;
private:
	bool _active = true;
	std::string _techniqueName;
	TechniqueType _techniqueType;
	std::vector<std::unique_ptr<Step>> _steps;
};

}
