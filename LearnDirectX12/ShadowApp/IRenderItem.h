#pragma once
#include <string>
#include <dx12lib/Context/ContextStd.h>

struct IRenderItem {
	virtual void draw(dx12lib::DirectContextProxy pDirectCtx) = 0;
	virtual ~IRenderItem() = default;
	std::string psoName;
};