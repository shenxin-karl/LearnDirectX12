#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef _DECLARE_LIGHT_
#define _DECLARE_LIGHT_
struct LightData {
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
struct MaterialData {
    float4 diffuseAlbedo;	// ������
    float  roughness;		// �ֲڶ�
    float  metallic;		// ������
    float  padding0;		// ���0
    float  padding1;		// ���1
};
#endif

#ifndef _DECLARE_SH3_
#define _DECLARE_SH3_
struct SH3 {
	float4 y0p0;
	float4 y1n1; float4 y1p0; float4 y1p1;
	float4 y2n2; float4 y2n1; float4 y2p0; float4 y2p1; float4 y2p2;
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

#ifndef kMaxLightCount 
	#define kMaxLightCount 16
#endif

struct CBLightType {
	int       directLightCount;
	int       pointLightCount;
	int       spotLightCount;
	int       objectPad0;
	float4	  ambientLight;
	LightData lights[kMaxLightCount];
};

struct CSMSubFrustum {
	float    width;					// ����ͶӰ��
	float    height;				// ����ͶӰ��	
	float    zNear;					// ����ͶӰ��
	float    zFar;					// ����ͶӰԶ
	float4x4 worldToLightMatrix;	// ��ռ����
	float3   center;				// view ��Դ����
	float	 lightPlane;			// ��Դƽ��, PCSS ʹ��
};

const static uint kMaxShadowCascaded = 7;
struct cbShadowType {
	float3		  lightDir;			// ��Դ����
	float		  lightSize;		// ���Դ��С, PCSS ʹ��
	CSMSubFrustum subFrustum[kMaxShadowCascaded];		
};

#ifndef	DISABLE_DEFAULT_SAMPLER
SamplerState gSamPointWrap					   : register(s0);
SamplerState gSamPointClamp					   : register(s1);
SamplerState gSamLinearWrap					   : register(s2);
SamplerState gSamLinearClamp				   : register(s3);
SamplerState gSamAnisotropicWrap			   : register(s4);
SamplerState gSamAnisotropicClamp			   : register(s5);
SamplerComparisonState gSamLinearShadowCompare : register(s6);
SamplerComparisonState gSamPointShadowCompare  : register(s7);
#endif // !DISABLE_DEFAULT_SAMPLER

#endif // !_COMMON_H_