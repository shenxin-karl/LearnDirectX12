#include "Camera.h"

namespace d3dUtil {

CameraBase::CameraBase(const CameraDesc &desc) {
	_lookFrom = desc.lookFrom;
	_lookUp = desc.lookUp;
	_lookAt = desc.lookAt;
	_nearClip = desc.nearClip;
	_farClip = desc.farClip;
	_fov = desc.fov;
	_aspect = desc.aspect;
}

CoronaCamera::CoronaCamera(const CoronaCameraDesc &desc) : CameraBase(desc) {
	_phi = desc.phi;
	_theta = desc.theta;
	_radius = desc.radius;
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

	DX::XMVECTOR lookAt = _lookAt.toVec();
	_lookFrom = float3(lookAt + direction.toVec() * _radius);
	float3 right = { _lookFrom.x, 0, _lookFrom.z };
	_lookUp = cross(-direction, right);

	DX::XMMATRIX view = DX::XMMatrixLookAtLH(_lookFrom.toVec(), _lookAt.toVec(), _lookUp.toVec());
	DX::XMMATRIX proj = DX::XMMatrixPerspectiveFovLH(DX::XMConvertToDegrees(_fov), _aspect, _nearClip, _farClip);
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

}