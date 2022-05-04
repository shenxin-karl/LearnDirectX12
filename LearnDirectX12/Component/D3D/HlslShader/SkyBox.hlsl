// #define USE_GAMMA	¿ªÆô gamma ½ÃÕý

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

TextureCube  gCubeMap       : register(t0);
SamplerState gSamLinearWrap : register(s0);

float4 PS(VertexOut pin) : SV_Target {
	float3 texColor = gCubeMap.Sample(gSamLinearWrap, pin.texcoord).rgb;
#ifndef USE_GAMMA
	return float4(texColor, 1.0);
#else
	return pow(float4(texColor, 1.0), 1.0 / 2.2);
#endif
}