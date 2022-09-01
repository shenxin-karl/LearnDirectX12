#pragma once
#include <DirectXCollision.h>
#include "Math/MathHelper.h"
#include "D3D/d3dutil.h"
#include "Math/MathStd.hpp"

namespace com {

struct MouseEvent;
struct KeyEvent;
class GameTimer;
enum class MouseState;

}

namespace d3d {

using namespace Math;

struct CameraDesc {
	float3 lookFrom;
	float3 lookUp;
	float3 lookAt;
	float  fov;
	float  nearClip;
	float  farClip;
	float  aspect;
};

class CameraBase {
public:
	CameraBase(const CameraDesc &desc);
	virtual const float4x4 &getView() const = 0;
	virtual const float4x4 &getProj() const = 0;
	virtual const float4x4 &getViewProj() const = 0;
	virtual	const float4x4 &getInvView() const = 0;
	virtual const float4x4 &getInvProj() const = 0;
	virtual const float4x4 &getInvViewProj() const = 0;
	virtual void update(std::shared_ptr<com::GameTimer> pGameTimer) = 0;
	Matrix4 getMatView() const;
	Matrix4 getMatProj() const;
	Matrix4 getMatViewProj() const;
	Matrix4 getMatInvView() const;
	Matrix4 getMatInvProj() const;
	Matrix4 getMatInvViewProj() const;
	void updatePassCB(d3d::CBPassType &passCB) const;
	void setFov(float fov);
	void setAspect(float aspect);
	float getFov() const;
	float getAspect() const;
	const Frustum &getProjSpaceFrustum() const;
	const Frustum &getViewSpaceFrustum() const;
protected:
	float  _fov;
	float  _aspect;
public:
	float3  _lookFrom;
	float3  _lookUp;
	float3  _lookAt;
	float   _nearClip;
	float   _farClip;
	Frustum _viewSpaceFrustum;
	Frustum _projSpaceFrustum;
#if defined(_DEBUG) || defined(DEBUG)
	bool   _isDirty = false;
#endif

};

}