#include "../../Component/D3D/shader/CBRegisterType.hlsl"
#include "../../Component/D3D/shader/LightingUtil.hlsl"
#include "../../Component/D3D/shader/PassConstantBuffer.hlsl"
#include "../../Component/D3D/shader/MaterialConstantBuffer.hlsl"

struct VertexIn {
	float3 position : POSITION;
	float4 color	: COLOR;
};

struct VertexOut {
	float4 svPosition : SV_POSITION;
	float4 color	  : COLOR;
};

cbuffer ObjectConstantBuffer : register(CB_OBJECT_TYPE) {
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