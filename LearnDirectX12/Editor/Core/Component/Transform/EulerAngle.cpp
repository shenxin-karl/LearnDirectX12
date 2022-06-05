#include "EulerAngle.h"
#include <algorithm>
#include <cmath>

namespace Math {

EulerAngle::EulerAngle(float pitch, float yaw, float roll) {
	setYaw(yaw);
	setPitch(pitch);
	setRoll(roll);
}

void EulerAngle::setYaw(float yaw) {
	_yaw = std::clamp(yaw, -179.9f, +180.f);
}

void EulerAngle::setPitch(float pitch) {
	_pitch = std::clamp(pitch, -89.9f, 90.f);
}

void EulerAngle::setRoll(float roll) {
	_roll = std::clamp(roll, -179.9f, 180.f);
}

float EulerAngle::getYaw() const {
	return _yaw;
}

float EulerAngle::getPitch() const {
	return _pitch;
}

float EulerAngle::getRoll() const {
	return _yaw;
}

Matrix3 EulerAngle::toMatrix3() const {
	Matrix3 res(DX::XMMatrixRotationRollPitchYaw(_pitch, _yaw, _roll));
	return res;
}

Matrix4 EulerAngle::toMatrix4() const {
	return DX::XMMatrixRotationRollPitchYaw(_pitch, _yaw, _roll);
}

float wrapPi(float theta) {
	constexpr float kPI = 3.141592654f;
	constexpr float k2PI = kPI * 2.f;
	if (std::abs(theta) <= kPI) {
		float revolutions = std::floor((theta + kPI) * (1.f / k2PI));
		theta -= revolutions * k2PI;
	}
	return theta;
}

EulerAngle lerp(const EulerAngle &lhs, EulerAngle &rhs, float t) {
	float dYaw = wrapPi(lhs.getYaw() - rhs.getYaw());
	float dPitch = wrapPi(lhs.getPitch() - rhs.getPitch());
	float dRoll = wrapPi(lhs.getRoll() - rhs.getRoll());
	return {
		lhs.getYaw() + t * dYaw,
		lhs.getPitch() + t * dPitch,
		lhs.getRoll() + t * dRoll,
	};
}

}
