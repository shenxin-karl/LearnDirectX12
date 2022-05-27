#pragma once
#include "Editor/IEditorItem.h"

class IComponent : public ED::IComponent {
public:
	IComponent(const IComponent &) = delete;
	IComponent &operator=(const IComponent &) = delete;
	virtual ~IComponent() = default;
};

