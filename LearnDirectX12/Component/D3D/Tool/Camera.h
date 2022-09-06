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


struct CameraDesc {
	Math::float3 lookFrom;
	Math::float3 lookUp;
	Math::float3 lookAt;
	float        fov;
	float        nearClip;
	float        farClip;
	float        aspect;
};

class CameraBase {
public:
	CameraBase(const CameraDesc &desc);
	virtual const Math::float4x4 &getView() const = 0;
	virtual const Math::float4x4 &getProj() const = 0;
	virtual const Math::float4x4 &getViewProj() const = 0;
	virtual	const Math::float4x4 &getInvView() const = 0;
	virtual const Math::float4x4 &getInvProj() const = 0;
	virtual const Math::float4x4 &getInvViewProj() const = 0;
	virtual void update(std::shared_ptr<com::GameTimer> pGameTimer) = 0;
	Math::Matrix4 getMatView() const;
	Math::Matrix4 getMatProj() const;
	Math::Matrix4 getMatViewProj() const;
	Math::Matrix4 getMatInvView() const;
	Math::Matrix4 getMatInvProj() const;
	Math::Matrix4 getMatInvViewProj() const;
	void updatePassCB(d3d::CBPassType &passCB) const;
	void setFov(float fov);
	void setAspect(float aspect);
	float getFov() const;
	float getAspect() const;
	const Math::BoundingFrustum &getProjSpaceFrustum() const;
	const Math::BoundingFrustum &getViewSpaceFrustum() const;
protected:
	float  _fov;
	float  _aspect;
public:
	Math::float3  _lookFrom;
	Math::float3  _lookUp;
	Math::float3  _lookAt;
	float         _nearClip;
	float         _farClip;
	Math::BoundingFrustum _viewSpaceFrustum;
	Math::BoundingFrustum _projSpaceFrustum;
#if defined(_DEBUG) || defined(DEBUG)
	bool   _isDirty = false;
#endif

};

}