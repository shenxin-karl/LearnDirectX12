/*
开启 gamma 矫正
#define USE_GAMMA
// 开启色彩映调
#define TONE_MAPPING	
*/

#ifndef GAMMA
	#define GAMMA 2.2
#endif

cbuffer CBSkyBoxSetting {
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

float3 ACESToneMapping(float3 color, float adaptedLum) {
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;
	color *= adaptedLum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}

TextureCube  gCubeMap       : register(t0);
SamplerState gSamLinearWrap : register(s0);

float4 PS(VertexOut pin) : SV_Target {
	float3 texColor = gCubeMap.Sample(gSamLinearWrap, pin.texcoord).rgb;

#ifdef USE_GAMMA
	return float4(texColor, 1.0);
#else
	texColor = ACESToneMapping(texColor, 1.0);
	texColor = pow(texColor.rgb, 1.0 / 2.2);
	return float4(texColor, 1.0);
#endif
}