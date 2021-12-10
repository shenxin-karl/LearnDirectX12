#pragma once
#include "Math/MathHelper.h"
#include "Math/VectorHelper.h"
//#include "Math/MatrixHelper.h"

namespace d3dUlti {
using namespace vec;

struct Light {
	float3	strength	= float3(0.5f);				// ��ķ���ǿ��
	float	falloffStar = 1.0f;						// ���Դ/�۹��ʹ��
	float3	direction	= float3(0.f, -1.f, 0.f);	// �����Դʹ��
	float	falloffEnd	= 10.f;						// ���Դ/�۹��ʹ��
	float3	position	= float3(0.f);				// ���Դ/�۹��ʹ��
	float	spotPower	= 64.f;						// �۹��ʹ��
};

}