#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef _DECLARE_LIGHT_
#define _DECLARE_LIGHT_
struct LightData {
	float3  strength;		// 辐射强度
	float   falloffStart;	// 点光源/聚光灯衰减开始距离
	float3  direction;		// 方向光方向
	float   falloffEnd;		// 点光源/聚光灯衰减结束距离
	float3  position;		// 点光源位置
	float   spotPower;		// 聚光灯 pow 指数
};
#endif

#ifndef _DECLARE_MATERIAL_
#define _DECLARE_MATERIAL_
struct MaterialData {
    float4 diffuseAlbedo;	// 反照率
    float  roughness;		// 粗糙度
    float  metallic;		// 金属度
    float  padding0;		// 填充0
    float  padding1;		// 填充1
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
	float    width;					// 正交投影宽
	float    height;				// 正交投影高	
	float    zNear;					// 正交投影近
	float    zFar;					// 正交投影远
	float4x4 worldToLightMatrix;	// 光空间矩阵
	float3   center;				// view 光源中心
	float	 lightPlane;			// 光源平面, PCSS 使用
};

const static uint kMaxShadowCascaded = 7;
struct cbShadowType {
	float3		  lightDir;			// 光源方向
	float		  lightSize;		// 面光源大小, PCSS 使用
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