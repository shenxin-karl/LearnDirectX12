#include "../../Component/D3D/shader/ShaderCommon.hlsl"

cbuffer CBPass : register(b0) {
	PassCBType gPassCB;
};

cbuffer CBLight : register(b1) {
	LightCBType gLight;
};

cbuffer CBObject : register(b2) {
	float4x4  gWorld;
	Material  gMaterial;
};

struct VertexIn {
	float3 position : POSITION;
	float3 normal   : NORMAL;
};

struct VertexOut {
	float4 svPosition : SV_POSITION;
	float3 wpos       : POSITION;
	float3 wnrm       : NORMAL;
};

VertexOut VS(VertexIn vin) {
	VertexOut vout;
	float4 worldPos = mul(gWorld, float4(vin.position, 1.0f));
	vout.svPosition = mul(gPassCB.viewProj, worldPos);
	vout.wpos = worldPos;
	vout.wnrm = vin.normal;
	return vout;
}

float4 PS(VertexOut pin) : SV_Target { 
	float3 N = normalize(pin.wnrm);
	float3 color = N * 0.5f + 0.5f;
	return float4(color, 1.0f);
}