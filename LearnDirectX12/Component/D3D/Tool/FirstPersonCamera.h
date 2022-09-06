#pragma once
#include <D3D/Tool/Camera.h>

namespace d3d {

class FirstPersonCamera : public CameraBase {
public:
	FirstPersonCamera(const CameraDesc &desc);
	const Math::float4x4 &getView() const override;
	const Math::float4x4 &getProj() const override;
	const Math::float4x4 &getViewProj() const override;
	const Math::float4x4 &getInvView() const override;
	const Math::float4x4 &getInvProj() const override;
	const Math::float4x4 &getInvViewProj() const override;
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

	enum MotionState {
		Forward = 0,
		backward = 1,
		Left = 2,
		Right = 3,
		LeftRotate = 4,
		RightRotate = 5,
	};
	void setMotionState(MotionState ms);
public:
	float _mouseWheelSensitivity = 1.f;
	float _mouseMoveSensitivity = 0.2f;		// degrees
	float _rollSensitivity = 15.f;			// degrees
	float _cameraMoveSpeed = 5.f;			// The number of positions moved in one second
private:
	void responseEvent(std::shared_ptr<com::GameTimer> pGameTimer);
private:
	float			_pitch;
	float			_yaw;
	float			_roll;
	Math::float4x4	_view;
	Math::float4x4	_proj;
	Math::float4x4	_viewProj;
	Math::float4x4	_invView;
	Math::float4x4	_invProj;
	Math::float4x4	_invViewProj;
	POINT			_lastMousePosition = POINT(-1, -1);
	bool			_moveState[6] = { false };
};

}
