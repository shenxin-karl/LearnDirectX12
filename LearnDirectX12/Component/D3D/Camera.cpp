#define NOMINMAX
#include <algorithm>
#include "Camera.h"

#include <iostream>

#include "D3D/ShaderCommon.h"
#include "GameTimer/GameTimer.h"
#include "InputSystem/Mouse.h"
#include "InputSystem/Keyboard.h"

namespace d3d {

CameraBase::CameraBase(const CameraDesc &desc) {
	float3 w = normalize(desc.lookAt - desc.lookFrom);
	float3 u = normalize(cross(desc.lookUp, w));
	float3 v = cross(w, u);

	_lookFrom = desc.lookFrom;
	_lookUp = v;
	_lookAt = desc.lookAt;
	_nearClip = desc.nearClip;
	_farClip = desc.farClip;
	_fov = desc.fov;
	_aspect = desc.aspect;

	assert(lengthSqr(_lookUp) > 0.f);
	assert(lengthSqr(w) > 0.f);
	assert(_fov > 1.f);
	assert(_nearClip > 0.f);
	assert(_farClip > _nearClip);
}

void CameraBase::updatePassCB(GPUStructuredCBPtr<d3d::PassCBType> pPassCB) const {
	assert(pPassCB != nullptr);
	auto pGPUPassCB = pPassCB->map();
	updatePassCB(*pGPUPassCB);
}

void CameraBase::updatePassCB(d3d::PassCBType &passCB) const {
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

CoronaCamera::CoronaCamera(const CameraDesc &desc) : CameraBase(desc) {
	auto direction = desc.lookAt - desc.lookFrom;
	_radius = length(direction);
	assert(_radius != 0.f);
	direction = -direction / _radius;
	_phi = DirectX::XMConvertToDegrees(std::asin(direction.y));
	_theta = DirectX::XMConvertToDegrees(std::atan2(direction.x, direction.z));
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
	float3 direction = {
		cosPhi * cosTh,
		sinPhi,
		cosPhi * sinTh,
	};

	float3 w = normalize(-direction);
	float3 u;
	if (std::abs(w.y) != 1.f)
		u = normalize(cross(float3(0, 1, 0), w));
	else
		u = float3(w.y > 0.f ? +1.f : -1.f, 0.f, 0.f);
	_lookUp = cross(w, u);
	_lookFrom = _lookAt + direction * _radius;

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(_lookFrom.toVec(), _lookAt.toVec(), _lookUp.toVec());
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(_fov), _aspect, _nearClip, _farClip);
	DirectX::XMMATRIX viewProj = view * proj;

	DirectX::XMVECTOR det;
	DirectX::XMMATRIX invView = DirectX::XMMatrixTranspose(view);
	det = DirectX::XMMatrixDeterminant(proj);
	DirectX::XMMATRIX invProj = DirectX::XMMatrixInverse(&det, proj);
	det = DirectX::XMMatrixDeterminant(viewProj);
	DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(&det, viewProj);

	DirectX::XMStoreFloat4x4(&_view, view);
	DirectX::XMStoreFloat4x4(&_proj, proj);
	DirectX::XMStoreFloat4x4(&_viewProj, viewProj);
	DirectX::XMStoreFloat4x4(&_invView, invView);
	DirectX::XMStoreFloat4x4(&_invProj, invProj);
	DirectX::XMStoreFloat4x4(&_invViewProj, invViewProj);
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
	_phi = std::clamp(phi, -89.f, +89.f);
}

void CoronaCamera::setTheta(float theta) {
	_theta = theta;
}

void CoronaCamera::setRadiuse(float radius) {
	_radius = std::clamp(radius, 0.001f, std::numeric_limits<float>::infinity());
}

void CoronaCamera::pollEvent(const com::MouseEvent &event) {
	switch (event.state_) {
	case com::MouseState::LPress: {
		_isMouseLeftPress = true;
		_lastMousePosition.x = event.x;
		_lastMousePosition.y = event.y;
		break;
	}
	case com::MouseState::LRelease: {
		_isMouseLeftPress = false;
		break;
	}
	case com::MouseState::Wheel: {
		auto radius = std::max(0.1f, getRadius() - event.offset_ * _mouseWheelSensitivity);
		setRadiuse(radius);
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
	auto target = normalize(desc.lookAt - desc.lookFrom);
	auto upDir = normalize(desc.lookUp);
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
	float3 target = {
		cosPitch * cosYaw,
		sinPitch,
		cosPitch * sinYaw,
	};
	_lookAt = normalize(target) + _lookFrom;

	float radianRoll = DirectX::XMConvertToRadians(_roll);
	float sinRoll = std::sin(radianRoll);
	float cosRoll = std::cos(radianRoll);
	_lookUp = float3(cosRoll, sinRoll, 0.f);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(_lookFrom.toVec(), _lookAt.toVec(), _lookUp.toVec());
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(_fov), _aspect, _nearClip, _farClip);
	DirectX::XMMATRIX viewProj = view * proj;

	DirectX::XMVECTOR det;
	DirectX::XMMATRIX invView = DirectX::XMMatrixTranspose(view);
	det = DirectX::XMMatrixDeterminant(proj);
	DirectX::XMMATRIX invProj = DirectX::XMMatrixInverse(&det, proj);
	det = DirectX::XMMatrixDeterminant(viewProj);
	DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(&det, viewProj);

	DirectX::XMStoreFloat4x4(&_view, view);
	DirectX::XMStoreFloat4x4(&_proj, proj);
	DirectX::XMStoreFloat4x4(&_viewProj, viewProj);
	DirectX::XMStoreFloat4x4(&_invView, invView);
	DirectX::XMStoreFloat4x4(&_invProj, invProj);
	DirectX::XMStoreFloat4x4(&_invViewProj, invViewProj);
}

void FirstPersonCamera::pollEvent(const com::MouseEvent &event) {
	if (_lastMousePosition.x == -1 && _lastMousePosition.y == -1) 
		_lastMousePosition = POINT(event.x, event.y);

	if (event.state_ == com::MouseState::Move) {
		float dx = static_cast<float>(event.x - _lastMousePosition.x) * _mouseMoveSensitivity;
		float dy = static_cast<float>(event.y - _lastMousePosition.y) * _mouseMoveSensitivity;
		if (dx != 0.f && dy != 0.f)
			std::cerr << "dx: " << dx << ", dy: " << dy << std::endl;
		setPitch(_pitch - dy);
		setYaw(_yaw - dx);
		_lastMousePosition = POINT(event.x, event.y);
	}
}

void FirstPersonCamera::pollEvent(const com::KeyEvent &event) {
	bool isPressed = event.getState() == com::KeyState::Pressed;
	switch (event.getKey()) {
	case 'W':
		_moveState[Forward] = isPressed;
		break;
	case 'S':
		_moveState[backward] = isPressed;
		break;
	case 'A':
		_moveState[Left] = isPressed;
		break;
	case 'D':
		_moveState[Right] = isPressed;
		break;
	case 'Q':
		_moveState[LeftRotate] = isPressed;
		break;
	case 'E':
		_moveState[RightRotate] = isPressed;
		break;
	}
}

void FirstPersonCamera::setPitch(float pitch) {
	_pitch = std::clamp(pitch, -89.9f, +89.9f);
}

void FirstPersonCamera::setYaw(float yaw) {
	_yaw = yaw;
}

void FirstPersonCamera::setRoll(float roll) {
	_roll = roll;
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
		auto w = normalize(_lookAt - _lookFrom);
		auto u = cross(_lookUp, w);
		float3 motor = normalize(w * advance + u * deviation) * (deltaTime * _cameraMoveSpeed);
		_lookFrom += motor;
	}

	if (rotate != 0.f)
		_roll += rotate * _rollSensitivity * deltaTime;
}

}
