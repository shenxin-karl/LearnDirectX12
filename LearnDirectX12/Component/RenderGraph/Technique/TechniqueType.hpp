#pragma once
#include <bitset>

namespace rg {

enum class TechniqueType : size_t {
	None,
	Color,
	Shadow,
	Count,
	AllSet = Count,
};

constexpr TechniqueType operator|(const TechniqueType &lhs, const TechniqueType &rhs) {
	return static_cast<TechniqueType>(static_cast<size_t>(lhs) | static_cast<size_t>(rhs));
}

constexpr TechniqueType operator&(const TechniqueType &lhs, const TechniqueType &rhs) {
	return static_cast<TechniqueType>(static_cast<size_t>(lhs) & static_cast<size_t>(rhs));
}

constexpr TechniqueType operator~(const TechniqueType &tt) {
	return static_cast<TechniqueType>(~static_cast<size_t>(tt));
}

constexpr TechniqueType operator^(const TechniqueType &lhs, const TechniqueType &rhs) {
	return static_cast<TechniqueType>(static_cast<size_t>(lhs) ^ static_cast<size_t>(rhs));
}

struct TechniqueFlag {
	TechniqueFlag(TechniqueType techniqueType) : _bits(0) {
		if (techniqueType == TechniqueType::AllSet)
			_bits.flip();
		else
			_bits.set(static_cast<size_t>(techniqueType));
	}

	TechniqueFlag &operator=(TechniqueType techniqueType) {
		TechniqueFlag tmp(techniqueType);
		swap(*this, tmp);
		return *this;
	}

	TechniqueFlag(const TechniqueFlag &) = default;
	TechniqueFlag(TechniqueFlag &&) = default;
	TechniqueFlag &operator=(const TechniqueFlag &) = default;
	TechniqueFlag &operator=(TechniqueFlag &&) = default;

	bool test(TechniqueType techniqueType) const {
		return _bits.test(static_cast<size_t>(techniqueType));
	}

	void reset() {
		_bits.reset();
	}

	void set(TechniqueType techniqueType) {
		_bits.set(static_cast<size_t>(techniqueType));
	}

	void flip() {
		_bits.flip();
	}

	friend void swap(TechniqueFlag &lhs, TechniqueFlag &rhs) {
		using std::swap;
		swap(lhs._bits, rhs._bits);
	}
private:
	constexpr static size_t kBitCount = static_cast<size_t>(TechniqueType::Count);
	std::bitset<kBitCount> _bits;
};

}
