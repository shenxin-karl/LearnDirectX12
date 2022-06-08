#pragma once

namespace com {

class IComponent {
public:
	IComponent(const IComponent &) = delete;
	IComponent &operator=(const IComponent &) = delete;
	virtual ~IComponent() = default;
};

}



