/*
开启 gamma 矫正
#define ENABLE_GAMMA_CORRECTION
// 开启色彩映调
#define ENABLE_TONE_MAPPING	
*/

#if defined(ENABLE_GAMMA_CORRECTION) || defined(ENABLE_TONE_MAPPING)
	#include "ColorGrading.hlsl"
#endif 

cbuffer CBSkyBoxSetting : register(b0) {
	float4x4 gViewProj;
};

struct VertexIn {
	float3 position : POSITION;
};

struct VertexOut {
	float4 SVPosition : SV_Position;
	float3 texcoord   : TEXCOORD;
};

VertexOut VS(VertexIn vin) {
	VertexOut vout;
	vout.SVPosition = mul(gViewProj, float4(vin.position, 1.0)).xyww;
	vout.texcoord   = vin.position;
	return vout;
}

TextureCube  gCubeMap       : register(t0);
SamplerState gSamLinearWrap : register(s0);

float4 PS(VertexOut pin) : SV_Target {
	float3 texColor = gCubeMap.Sample(gSamLinearWrap, pin.texcoord).rgb;

#ifdef ENABLE_TONE_MAPPING
	texColor = ACESToneMapping(texColor, 1.0);
#endif

#ifdef ENABLE_GAMMA_CORRECTION
	texColor = GammaCorrection(texColor);
#endif
	return float4(texColor, 1.0);
}