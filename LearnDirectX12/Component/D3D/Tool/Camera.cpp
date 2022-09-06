#define NOMINMAX
#include <algorithm>
#include "Camera.h"

#include <iostream>

#include "D3D/Shader/ShaderCommon.h"
#include "GameTimer/GameTimer.h"
#include "InputSystem/Mouse.h"
#include "InputSystem/Keyboard.h"

namespace d3d {

using namespace Math;

#if defined(_DEBUG) || defined(DEBUG)
	#define DEBUG_MAKE_DIRTY  _isDirty = true
	#define DEBUG_CLEAR_DIRTY _isDirty = false
#else
	#define DEBUG_MAKE_DIRTY  (void)0
	#define DEBUG_CLEAR_DIRTY (void)0
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

const BoundingFrustum &CameraBase::getProjSpaceFrustum() const {
	return _projSpaceFrustum;
}

const BoundingFrustum &CameraBase::getViewSpaceFrustum() const {
	return _viewSpaceFrustum;
}

}
