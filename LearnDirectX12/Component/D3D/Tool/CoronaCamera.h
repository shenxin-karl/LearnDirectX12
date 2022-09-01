#pragma once
#include <D3D/Tool/Camera.h>

namespace d3d {

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

}
