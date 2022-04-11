#pragma once
#include "Math/MathHelper.h"
#include "D3D/d3dutil.h"

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
	void updatePassCB(d3d::PassCBType &passCB) const;
	void setFov(float fov);
	void setAspect(float aspect);
	float getFov() const;
	float getAspect() const;
protected:
	float  _fov;
	float  _aspect;
public:
	float3 _lookFrom;
	float3 _lookUp;
	float3 _lookAt;
	float  _nearClip;
	float  _farClip;
};


class CoronaCamera : public CameraBase {
public:
	CoronaCamera(const CameraDesc &desc);
	const float4x4 &getView() const override;
	const float4x4 &getProj() const override;
	const float4x4 &getViewProj() const override;
	const float4x4 &getInvView() const override;
	const float4x4 &getInvProj() const override;
	const float4x4 &getInvViewProj() const override;
	void update(std::shared_ptr<com::GameTimer> pGameTimer) override;
	float getPhi() const;
	float getTheta() const;
	float getRadius() const;
	void setPhi(float phi);
	void setTheta(float theta);
	void setRadius(float radius);
	void pollEvent(const com::MouseEvent &event);
public:
	float _mouseWheelSensitivity = 0.1f;
	float _moveSensitivity = 0.5f;
private:
	float    _phi;
	float    _theta;
	float    _radius;
	float4x4 _view;
	float4x4 _proj;
	float4x4 _viewProj;
	float4x4 _invView;
	float4x4 _invProj;
	float4x4 _invViewProj;
	bool     _isMouseLeftPress = false;
	POINT    _lastMousePosition;
};


class FirstPersonCamera : public CameraBase {
public:
	FirstPersonCamera(const CameraDesc &desc);
	const float4x4 &getView() const override;
	const float4x4 &getProj() const override;
	const float4x4 &getViewProj() const override;
	const float4x4 &getInvView() const override;
	const float4x4 &getInvProj() const override;
	const float4x4 &getInvViewProj() const override;
	void update(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void pollEvent(const com::MouseEvent &event);
	void pollEvent(const com::KeyEvent &event);
	void setPitch(float pitch);
	void setYaw(float yaw);
	void setRoll(float roll);
	void setLastMousePosition(POINT pos);
	float getPitch() const;
	float getYaw() const;
	float getRoll() const;
	POINT getLastMousePosition() const;
public:
	float _mouseWheelSensitivity = 1.f;		
	float _mouseMoveSensitivity = 0.2f;		// degrees
	float _rollSensitivity = 15.f;			// degrees
	float _cameraMoveSpeed = 5.f;			// The number of positions moved in one second
private:
	enum {
		Forward     = 0,
		backward    = 1,
		Left        = 2,
		Right       = 3,
		LeftRotate  = 4,
		RightRotate = 5,
	};
	void responseEvent(std::shared_ptr<com::GameTimer> pGameTimer);
private:
	float    _pitch;
	float    _yaw;
	float    _roll;
	float4x4 _view;
	float4x4 _proj;
	float4x4 _viewProj;
	float4x4 _invView;
	float4x4 _invProj;
	float4x4 _invViewProj;
	POINT    _lastMousePosition = POINT(-1, -1);
	bool     _moveState[6] = { false };
};

}