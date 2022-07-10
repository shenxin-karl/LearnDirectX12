#pragma once

namespace rg {

struct TechniqueType {
	enum Type : size_t {
		None  = (0 << 0),
		Color = (1 << 0),
	};
public:
	TechniqueType(Type type) : _type(type) {}
	TechniqueType(const TechniqueType &) = default;
	TechniqueType(TechniqueType &&) = default;
	TechniqueType &operator=(const TechniqueType &) = default;
	TechniqueType &operator=(TechniqueType &&) = default;
	~TechniqueType() = default;

	operator Type() const noexcept {
		return _type;
	}

	operator bool() const {
		return _type != None;
	}

	friend TechniqueType operator|(const TechniqueType &lhs, const TechniqueType &rhs) {
		return static_cast<Type>(lhs._type | rhs._type);
	}

	friend TechniqueType operator&(const TechniqueType &lhs, const TechniqueType &rhs) {
		return static_cast<Type>(lhs._type & rhs._type);
	}
private:
	Type _type = None;
};

}
