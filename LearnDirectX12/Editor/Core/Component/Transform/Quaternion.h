#pragma once
#include "Math/MathStd.hpp"

namespace Math {

class Quaternion {
public:
	Quaternion(float w = 1.f, float x = 0.f, float y = 0.f, float z = 0.f);
private:
	union {
		struct {
			float x;
			float y;
			float z;
			float w;
		};
		Vector4 _vec;
	};
};
	
}
