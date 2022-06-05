#pragma once
#include "Component/IComponent.h"
#include "Math/MathStd.hpp"

namespace Math {


// Roll Pitch  Yaw
// Bank Pitch  Heading
class EulerAngle {
public:
	EulerAngle(float pitch, float yaw, float roll);
	void setYaw(float yaw);
	void setPitch(float pitch);
	void setRoll(float roll);
	float getYaw() const;
	float getPitch() const;
	float getRoll() const;
	Matrix3 toMatrix3() const;
	Matrix4 toMatrix4() const;
private:
	float _pitch = 90.f;
	float _yaw   = 0.f;
	float _roll  = 0.f;
};


float wrapPi(float theta);

EulerAngle lerp(const EulerAngle &lhs, EulerAngle &rhs, float t);

}
