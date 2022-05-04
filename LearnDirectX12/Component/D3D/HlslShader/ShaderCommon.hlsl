#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef _DECLARE_LIGHT_
#define _DECLARE_LIGHT_
struct Light {
	float3  strength;		// ����ǿ��
	float   falloffStart;	// ���Դ/�۹��˥����ʼ����
	float3  direction;		// ����ⷽ��
	float   falloffEnd;		// ���Դ/�۹��˥����������
	float3  position;		// ���Դλ��
	float   spotPower;		// �۹�� pow ָ��
};
#endif

#ifndef _DECLARE_MATERIAL_
#define _DECLARE_MATERIAL_
struct Material {
    float4 diffuseAlbedo;	// ������
    float  roughness;		// �ֲڶ�
    float  metallic;		// ������
    float  padding0;		// ���0
    float  padding1;		// ���1
};
#endif

struct CBPassType {
	float4x4 view;
	float4x4 invView;
	float4x4 proj;
	float4x4 invProj;
	float4x4 viewProj;
	float4x4 invViewProj;
	float3	 eyePos;
    float	 cbPerPassPad0; // padding
	float2	 renderTargetSize;
	float2	 invRenderTargetSize;
	float	 nearZ;
	float	 farZ;
	float	 totalTime;
	float	 deltaTime;
	float4   fogColor;
	float    fogStart;
    float    fogEnd;
	float2   cbPerPassPad1;
};

struct SH3 {
	float3 y00;
};

#define kMaxLightCount 16
struct CBLightType {
	int      directLightCount;
	int      pointLightCount;
	int      spotLightCount;
	int      objectPad0;
	float4	 ambientLight;
	Light    lights[kMaxLightCount];
};

#ifndef	DISABLE_DEFAULT_SAMPLER
SamplerState gSamPointWrap        : register(s0);
SamplerState gSamPointClamp       : register(s1);
SamplerState gSamLinearWrap       : register(s2);
SamplerState gSamLinearClamp      : register(s3);
SamplerState gSamAnisotropicWrap  : register(s4);
SamplerState gSamAnisotropicClamp : register(s5);
#endif // !DISABLE_DEFAULT_SAMPLER

#endif // !_COMMON_H_