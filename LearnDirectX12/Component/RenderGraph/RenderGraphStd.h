#pragma once

namespace rg {

#ifndef interface
#define interface
#endif

class Technique;
class Drawable;
class Bindable;
class Step;
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
