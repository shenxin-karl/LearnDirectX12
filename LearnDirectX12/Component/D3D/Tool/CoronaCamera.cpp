#include "CoronaCamera.h"
#include "InputSystem/Mouse.h"

namespace d3d {

#if defined(_DEBUG) || defined(DEBUG)
	#define DEBUG_MAKE_DIRTY  _isDirty = true
	#define DEBUG_CLEAR_DIRTY _isDirty = false
#else
	#define DEBUG_MAKE_DIRTY  (void)0
	#define DEBUG_CLEAR_DIRTY (void)0
#endif

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
	_projSpaceFrustum = { proj };
	_viewSpaceFrustum = _projSpaceFrustum.transform(invView);
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

}
