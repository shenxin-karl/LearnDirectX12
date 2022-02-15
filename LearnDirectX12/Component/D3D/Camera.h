#pragma once
#include "Math/MathHelper.h"

namespace d3dUtil {

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
public:
	float3 _lookFrom;
	float3 _lookUp;
	float3 _lookAt;
	float  _nearClip;
	float  _farClip;
	float  _fov;
	float  _aspect;
};


struct CoronaCameraDesc : public CameraDesc {
	float phi = 0.f;
	float theta = 0.f;
	float radius = 10.f;
};

class CoronaCamera : public CameraBase {
public:
	CoronaCamera(const CoronaCameraDesc &desc);
	virtual const float4x4 &getView() const override;
	virtual const float4x4 &getProj() const override;
	virtual const float4x4 &getViewProj() const override;
	virtual	const float4x4 &getInvView() const override;
	virtual const float4x4 &getInvProj() const override;
	virtual const float4x4 &getInvViewProj() const override;
	void update();
	float getPhi() const;
	float getTheta() const;
	float getRadius() const;
public:
	float _phi;
	float _theta;
	float _radius;
private:
	float4x4 _view;
	float4x4 _proj;
	float4x4 _viewProj;
	float4x4 _invView;
	float4x4 _invProj;
	float4x4 _invViewProj;
};

}