#define NOMINMAX
#include <algorithm>
#include "Camera.h"
#include "D3D/ShaderCommon.h"
#include "InputSystem/Mouse.h"

namespace d3dutil {

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
}

void CameraBase::updatePassCB(GPUStructCBPtr<d3dutil::PassCBType> pPassCB) const {
	auto pGPUPassCB = pPassCB->map();
	pGPUPassCB->view = getView();
	pGPUPassCB->invView = getInvView();
	pGPUPassCB->proj = getProj();
	pGPUPassCB->invProj = getInvProj();
	pGPUPassCB->viewProj = getViewProj();
	pGPUPassCB->invViewProj = getInvViewProj();
	pGPUPassCB->eyePos = _lookFrom;
	pGPUPassCB->nearZ = _nearClip;
	pGPUPassCB->farZ = _farClip;
}

CoronaCamera::CoronaCamera(const CameraDesc &desc) : CameraBase(desc) {
	auto direction = desc.lookAt - desc.lookFrom;
	_radius = length(direction);
	assert(_radius != 0.f);
	direction = -direction / _radius;
	_phi = DX::XMConvertToDegrees(std::asin(direction.y));
	_theta = DX::XMConvertToDegrees(std::atan2(direction.x, direction.z));
	update();
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

void CoronaCamera::update() {
	float phiRadians = DX::XMConvertToRadians(_phi);
	float thetaRadians = DX::XMConvertToRadians(_theta);
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

	DX::XMMATRIX view = DX::XMMatrixLookAtLH(_lookFrom.toVec(), _lookAt.toVec(), _lookUp.toVec());
	DX::XMMATRIX proj = DX::XMMatrixPerspectiveFovLH(DX::XMConvertToRadians(_fov), _aspect, _nearClip, _farClip);
	DX::XMMATRIX viewProj = view * proj;

	DX::XMVECTOR det;
	DX::XMMATRIX invView = DX::XMMatrixTranspose(view);
	det = DX::XMMatrixDeterminant(proj);
	DX::XMMATRIX invProj = DX::XMMatrixInverse(&det, proj);
	det = DX::XMMatrixDeterminant(viewProj);
	DX::XMMATRIX invViewProj = DX::XMMatrixInverse(&det, viewProj);

	DX::XMStoreFloat4x4(&_view, view);
	DX::XMStoreFloat4x4(&_proj, proj);
	DX::XMStoreFloat4x4(&_viewProj, viewProj);
	DX::XMStoreFloat4x4(&_invView, invView);
	DX::XMStoreFloat4x4(&_invProj, invProj);
	DX::XMStoreFloat4x4(&_invViewProj, invViewProj);
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
	_radius = radius;
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
		auto radius = std::max(0.1f, getRadius() - event.offset_ * _whellSensitivety);
		setRadiuse(radius);
		break;
	}
	case com::MouseState::Move: {
		if (_isMouseLeftPress) {
			float dx = static_cast<float>(event.x - _lastMousePosition.x) * _moveSensitivety;
			float dy = static_cast<float>(event.y - _lastMousePosition.y) * _moveSensitivety;
			setPhi(_phi + dy);
			setTheta(_theta - dx);
		}
		_lastMousePosition = POINT(event.x, event.y);
		break;
	}
	}
}

}