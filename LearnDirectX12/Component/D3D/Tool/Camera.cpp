#define NOMINMAX
#include <algorithm>
#include "Camera.h"

#include <iostream>

#include "D3D/Shader/ShaderCommon.h"
#include "GameTimer/GameTimer.h"
#include "InputSystem/Mouse.h"
#include "InputSystem/Keyboard.h"

namespace d3d {

#if defined(_DEBUG) || defined(DEBUG)
	#define DEBUG_MAKE_DIRTY  _isDirty = true
	#define DEBUG_CLEAR_DIRTY _isDirty = false
#else
	#define DEBUG_MAKE_DIRTY  (void)
	#define DEBUG_CLEAR_DIRTY (void)
#endif

CameraBase::CameraBase(const CameraDesc &desc) {
	Vector3 w = normalize(Vector3(desc.lookAt) - Vector3(desc.lookFrom));
	Vector3 u = normalize(cross(Vector3(desc.lookUp), w));
	Vector3 v = cross(w, u);

	_lookFrom = desc.lookFrom;
	_lookUp = v.xyz;
	_lookAt = desc.lookAt;
	_nearClip = desc.nearClip;
	_farClip = desc.farClip;
	_fov = desc.fov;
	_aspect = desc.aspect;

	assert(lengthSquare(Vector3(_lookUp)) > 0.f);
	assert(lengthSquare(w) > 0.f);
	assert(_fov > 1.f);
	assert(_nearClip > 0.f);
	assert(_farClip > _nearClip);
}

Matrix4 CameraBase::getMatView() const {
	return Matrix4(getView());
}

Matrix4 CameraBase::getMatProj() const {
	return Matrix4(getProj());
}

Matrix4 CameraBase::getMatViewProj() const {
	return Matrix4(getViewProj());
}

Matrix4 CameraBase::getMatInvView() const {
	return Matrix4(getInvView());
}

Matrix4 CameraBase::getMatInvProj() const {
	return Matrix4(getInvProj());
}

Matrix4 CameraBase::getMatInvViewProj() const {
	return Matrix4(getInvViewProj());
}

void CameraBase::updatePassCB(d3d::CBPassType &passCB) const {
	assert(!_isDirty);
	passCB.view = getView();
	passCB.invView = getInvView();
	passCB.proj = getProj();
	passCB.invProj = getInvProj();
	passCB.viewProj = getViewProj();
	passCB.invViewProj = getInvViewProj();
	passCB.eyePos = _lookFrom;
	passCB.nearZ = _nearClip;
	passCB.farZ = _farClip;
}

void CameraBase::setFov(float fov) {
	DEBUG_MAKE_DIRTY;
	_fov = std::clamp(fov, 1.f, 89.f);
}

void CameraBase::setAspect(float aspect) {
	DEBUG_MAKE_DIRTY;
	_aspect = aspect;
}

float CameraBase::getFov() const {
	return _fov;
}

float CameraBase::getAspect() const {
	return _aspect;
}

Frustum CameraBase::getProjSpaceFrustum() const {
	return { Matrix4(getProj())};
}

Frustum CameraBase::getViewSpaceFrustum() const {
	auto projSpaceFrustum = getProjSpaceFrustum();
	Matrix4 invView = Matrix4(getInvView());
	return projSpaceFrustum.transform(invView);;
}

CoronaCamera::CoronaCamera(const CameraDesc &desc) : CameraBase(desc) {
	auto direction = Vector3(desc.lookAt) - Vector3(desc.lookFrom);
	_radius = length(direction);
	assert(_radius != 0.f);
	direction = -direction / _radius;
	_phi = DirectX::XMConvertToDegrees(std::asin(direction.y));
	_theta = DirectX::XMConvertToDegrees(std::atan2(direction.z, direction.x));
}

const float4x4 &CoronaCamera::getView() const {
	return _view;
}

const float4x4 &CoronaCamera::getProj() const {
	return _proj;
}

const float4x4 &CoronaCamera::getViewProj() const {
	return _viewProj;
}

const float4x4 &CoronaCamera::getInvView() const {
	return _invView;
}

const float4x4 &CoronaCamera::getInvProj() const {
	return _invProj;
}

const float4x4 &CoronaCamera::getInvViewProj() const {
	return _invViewProj;
}

void CoronaCamera::update(std::shared_ptr<com::GameTimer> pGameTimer) {
	float phiRadians = DirectX::XMConvertToRadians(_phi);
	float thetaRadians = DirectX::XMConvertToRadians(_theta);
	float cosTh = std::cos(thetaRadians);
	float sinTh = std::sin(thetaRadians);
	float cosPhi = std::cos(phiRadians);
	float sinPhi = std::sin(phiRadians);
	Vector3 direction = {
		cosPhi * cosTh,
		sinPhi,
		cosPhi * sinTh,
	};

	Vector3 w = normalize(direction);
	Vector3 u;
	if (std::abs(w.y) != 1.f)
		u = normalize(cross(Vector3(0, 1, 0), w));
	else
		u = Vector3(w.y > 0.f ? +1.f : -1.f, 0.f, 0.f);
	Vector3 lookUp = cross(w, u);
	Vector3 lookAt = Vector3(_lookAt);
	Vector3 lookFrom = lookAt + direction * _radius;

	_lookFrom = lookFrom.xyz;
	_lookUp = lookUp.xyz;
	
	Matrix4 view = DirectX::XMMatrixLookAtLH(lookFrom, lookAt, lookUp);
	Matrix4 proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(_fov), _aspect, _nearClip, _farClip);
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
	DEBUG_CLEAR_DIRTY;
}

float CoronaCamera::getPhi() const {
	return _phi;
}

float CoronaCamera::getTheta() const {
	return _theta;
}

float CoronaCamera::getRadius() const {
	return _radius;
}

void CoronaCamera::setPhi(float phi) {
	DEBUG_MAKE_DIRTY;
	_phi = std::clamp(phi, -89.f, +89.f);
}

void CoronaCamera::setTheta(float theta) {
	DEBUG_MAKE_DIRTY;
	_theta = theta;
}

void CoronaCamera::setRadius(float radius) {
	DEBUG_MAKE_DIRTY;
	_radius = std::clamp(radius, 0.001f, std::numeric_limits<float>::infinity());
}

void CoronaCamera::pollEvent(const com::MouseEvent &event) {
	switch (event._state) {
	case com::MouseState::LPress: {
		_isMouseLeftPress = true;
		_lastMousePosition.x = event.x;
		_lastMousePosition.y = event.y;
		DEBUG_MAKE_DIRTY;
		break;
	}
	case com::MouseState::LRelease: {
		_isMouseLeftPress = false;
		break;
	}
	case com::MouseState::Wheel: {
		auto radius = std::max(0.1f, getRadius() - event._offset * _mouseWheelSensitivity);
		setRadius(radius);
		break;
	}
	case com::MouseState::Move: {
		if (_isMouseLeftPress) {
			float dx = static_cast<float>(event.x - _lastMousePosition.x) * _moveSensitivity;
			float dy = static_cast<float>(event.y - _lastMousePosition.y) * _moveSensitivity;
			setPhi(_phi + dy);
			setTheta(_theta - dx);
		}
		_lastMousePosition = POINT(event.x, event.y);
		break;
	}
	}
}

/******************************************************************************************/

FirstPersonCamera::FirstPersonCamera(const CameraDesc &desc) : CameraBase(desc) {
	Vector3 lookAt = Vector3(desc.lookAt);
	Vector3 lookFrom = Vector3(desc.lookFrom);
	Vector3 lookUp = Vector3(desc.lookUp);
	Vector3 target = normalize(lookAt - lookFrom);
	Vector3 upDir = normalize(lookUp);

	_pitch = std::clamp(DirectX::XMConvertToDegrees(std::asin(target.y)), -89.9f, +89.9f);
	_yaw = DirectX::XMConvertToDegrees(std::atan2(target.x, target.z));
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
	Matrix4 proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(_fov), _aspect, _nearClip, _farClip);
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
	} else if (event._state == com::MouseState::Wheel) {
		float fovDeviation = event._offset * _mouseWheelSensitivity;
		setFov(getFov() - fovDeviation);
	}
}

void FirstPersonCamera::pollEvent(const com::KeyEvent &event) {
	bool isPressed = event.getState() == com::KeyState::Pressed;
	switch (event.getKey()) {
	case 'W':
		_isDirty = true;
		_moveState[Forward] = isPressed;
		break;
	case 'S':
		_isDirty = true;
		_moveState[backward] = isPressed;
		break;
	case 'A':
		_isDirty = true;
		_moveState[Left] = isPressed;
		break;
	case 'D':
		_isDirty = true;
		_moveState[Right] = isPressed;
		break;
	case 'Q':
		_isDirty = true;
		_moveState[LeftRotate] = isPressed;
		break;
	case 'E':
		_isDirty = true;
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
