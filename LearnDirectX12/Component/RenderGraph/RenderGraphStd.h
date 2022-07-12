#pragma once

namespace rg {

class Technique;
class Drawable;
class Bindable;
class Step;
class SubPass;
class RenderQueuePass;
class Job;

class GraphicsPSOBindable;

struct TechniqueType;
struct DrawArgs;

class NonCopyable {
public:
	NonCopyable() = default;
	NonCopyable(const NonCopyable &) = delete;
	NonCopyable &operator=(const NonCopyable &) = delete;
	virtual ~NonCopyable() = default;
};


}
