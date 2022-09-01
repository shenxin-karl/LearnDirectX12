#include "FirstPersonCamera.h"

#include <iostream>

#include "GameTimer/GameTimer.h"
#include "InputSystem/Keyboard.h"
#include "InputSystem/Mouse.h"

#if defined(_DEBUG) || defined(DEBUG)
	#define DEBUG_MAKE_DIRTY  _isDirty = true
	#define DEBUG_CLEAR_DIRTY _isDirty = false
#else
	#define DEBUG_MAKE_DIRTY  (void)0
	#define DEBUG_CLEAR_DIRTY (void)0
#endif

namespace d3d {


FirstPersonCamera::FirstPersonCamera(const CameraDesc &desc) : CameraBase(desc) {
	Vector3 lookAt = Vector3(desc.lookAt);
	Vector3 lookFrom = Vector3(desc.lookFrom);
	Vector3 lookUp = Vector3(desc.lookUp);
	Vector3 target = normalize(lookAt - lookFrom);
	Vector3 upDir = normalize(lookUp);

	_pitch = std::clamp(DirectX::XMConvertToDegrees(std::asin(target.y)), -89.9f, +89.9f);
	_yaw = DirectX::XMConvertToDegrees(std::atan2(target.z, target.x));
	_roll = DirectX::XMConvertToDegrees(std::asin(upDir.y));
}

const float4x4 &FirstPersonCamera::getView() const {
	return _view;
}

const float4x4 &FirstPersonCamera::getProj() const {
	return _proj;
}

const float4x4 &FirstPersonCamera::getViewProj() const {
	return _viewProj;
}

const float4x4 &FirstPersonCamera::getInvView() const {
	return _invView;
}

const float4x4 &FirstPersonCamera::getInvProj() const {
	return _invProj;
}

const float4x4 &FirstPersonCamera::getInvViewProj() const {
	return _invViewProj;
}

void FirstPersonCamera::update(std::shared_ptr<com::GameTimer> pGameTimer) {
	responseEvent(pGameTimer);

	float radianPitch = DirectX::XMConvertToRadians(_pitch);
	float radianYaw = DirectX::XMConvertToRadians(_yaw);

	float sinPitch = std::sin(radianPitch);
	float cosPitch = std::cos(radianPitch);
	float sinYaw = std::sin(radianYaw);
	float cosYaw = std::cos(radianYaw);
	Vector3 target = {
		cosPitch * cosYaw,
		sinPitch,
		cosPitch * sinYaw,
	};

	Vector3 lookFrom = Vector3(_lookFrom);
	Vector3 lookAt = normalize(target) + lookFrom;

	float radianRoll = DirectX::XMConvertToRadians(_roll);
	float sinRoll = std::sin(radianRoll);
	float cosRoll = std::cos(radianRoll);
	Vector3 lookUp = Vector3(cosRoll, sinRoll, 0.f);

	_lookAt = lookAt.xyz;
	_lookUp = lookUp.xyz;

	Matrix4 view = DirectX::XMMatrixLookAtLH(lookFrom, lookAt, lookUp);
	Matrix4 proj = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(_fov), 
		_aspect, 
		_nearClip, 
		_farClip
	);
	Matrix4 viewProj = proj * view;

	Matrix4 invView = inverse(view);
	Matrix4 invProj = inverse(proj);
	Matrix4 invViewProj = inverse(viewProj);

	_view = float4x4(view);
	_proj = float4x4(proj);
	_viewProj = float4x4(viewProj);
	_invView = float4x4(invView);
	_invProj = float4x4(invProj);
	_invViewProj = float4x4(invViewProj);
	_projSpaceFrustum = { proj };
	_viewSpaceFrustum = _projSpaceFrustum.transform(invView);
	DEBUG_CLEAR_DIRTY;
}

void FirstPersonCamera::pollEvent(const com::MouseEvent &event) {
	if (_lastMousePosition.x == -1 && _lastMousePosition.y == -1)
		_lastMousePosition = POINT(event.x, event.y);

	if (event._state == com::MouseState::Move) {
		float dx = static_cast<float>(event.x - _lastMousePosition.x) * _mouseMoveSensitivity;
		float dy = static_cast<float>(event.y - _lastMousePosition.y) * _mouseMoveSensitivity;
		setPitch(_pitch - dy);
		setYaw(_yaw - dx);
		_lastMousePosition = POINT(event.x, event.y);
	}
	else if (event._state == com::MouseState::Wheel) {
		float fovDeviation = event._offset * _mouseWheelSensitivity;
		setFov(getFov() - fovDeviation);
	}
}

void FirstPersonCamera::pollEvent(const com::KeyEvent &event) {
	bool isPressed = event.getState() == com::KeyState::Pressed;
	switch (event.getKey()) {
	case 'W':
		DEBUG_MAKE_DIRTY;
		_moveState[Forward] = isPressed;
		break;
	case 'S':
		DEBUG_MAKE_DIRTY;
		_moveState[backward] = isPressed;
		break;
	case 'A':
		DEBUG_MAKE_DIRTY;
		_moveState[Left] = isPressed;
		break;
	case 'D':
		DEBUG_MAKE_DIRTY;
		_moveState[Right] = isPressed;
		break;
	case 'Q':
		DEBUG_MAKE_DIRTY;
		_moveState[LeftRotate] = isPressed;
		break;
	case 'E':
		DEBUG_MAKE_DIRTY;
		_moveState[RightRotate] = isPressed;
		break;
	}
}

void FirstPersonCamera::setPitch(float pitch) {
	DEBUG_MAKE_DIRTY;
	_pitch = std::clamp(pitch, -89.9f, +89.9f);
}

void FirstPersonCamera::setYaw(float yaw) {
	DEBUG_MAKE_DIRTY;
	_yaw = yaw;
}

void FirstPersonCamera::setRoll(float roll) {
	DEBUG_MAKE_DIRTY;
	_roll = roll;
}

void FirstPersonCamera::setLastMousePosition(POINT pos) {
	DEBUG_MAKE_DIRTY;
	_lastMousePosition = pos;
}

float FirstPersonCamera::getPitch() const {
	return _pitch;
}

float FirstPersonCamera::getYaw() const {
	return _yaw;
}

float FirstPersonCamera::getRoll() const {
	return _roll;
}

POINT FirstPersonCamera::getLastMousePosition() const {
	return _lastMousePosition;
}

void FirstPersonCamera::setMotionState(MotionState ms) {
	DEBUG_MAKE_DIRTY;
	_moveState[ms] = true;
}

void FirstPersonCamera::responseEvent(std::shared_ptr<com::GameTimer> pGameTimer) {
	float deltaTime = pGameTimer->getDeltaTime();
	float advance = 0.f;
	float deviation = 0.f;
	float rotate = 0.f;
	advance += static_cast<float>(_moveState[Forward]);
	advance -= static_cast<float>(_moveState[backward]);
	deviation += static_cast<float>(_moveState[Right]);
	deviation -= static_cast<float>(_moveState[Left]);
	rotate += static_cast<float>(_moveState[RightRotate]);
	rotate -= static_cast<float>(_moveState[LeftRotate]);

	if (advance != 0.f || deviation != 0.f) {
		Vector3 lookFrom = Vector3(_lookFrom);
		Vector3 lookAt = Vector3(_lookAt);
		Vector3 lookUp = Vector3(_lookUp);

		Vector3 w = normalize(lookAt - lookFrom);
		Vector3 u = cross(lookUp, w);
		Vector3 motor = normalize(w * advance + u * deviation) * (deltaTime * _cameraMoveSpeed);
		lookFrom += motor;
		_lookFrom = lookFrom.xyz;
	}

	if (rotate != 0.f)
		_roll += rotate * _rollSensitivity * deltaTime;
}

}
