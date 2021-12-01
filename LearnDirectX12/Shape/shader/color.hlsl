#include "../../Component/D3D/shader/PassConstantBuffer.hlsl"

struct VertexIn {
	float3 position : POSITION;
	float4 color	: COLOR;
};

struct VertexOut {
	float4 svPosition : SV_POSITION;
	float4 color	  : COLOR;
};

cbuffer ObjectConstantBuffer : register(b1) {
	float4x4 gWorld;
};

VertexOut VS(VertexIn vin) {
	VertexOut vout;
	float4 worldPos = mul(gWorld, float4(vin.position, 1.0));
	vout.svPosition = mul(gViewProj, worldPos);
	vout.color     = vin.color;
	return vout;
}

float4 PS(VertexOut pin) : SV_Target {
	return pin.color;
}