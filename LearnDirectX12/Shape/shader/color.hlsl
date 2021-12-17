#include "../../Component/D3D/shader/CBRegisterType.hlsl"
#include "../../Component/D3D/shader/LightingUtil.hlsl"
#include "../../Component/D3D/shader/PassConstantBuffer.hlsl"
#include "../../Component/D3D/shader/MaterialConstantBuffer.hlsl"

struct VertexIn {
	float3 position : POSITION;
    float3 normal   : NORMAL;
};

struct VertexOut {
	float4 svPosition : SV_POSITION;
	float3 wpos		  : POSITION;
	float3 wnrm		  : NORMAL;
};

cbuffer ObjectConstantBuffer : register(CB_OBJECT_TYPE) {
	float4x4 gWorld;
};

VertexOut VS(VertexIn vin) {
	VertexOut vout;
	float4 worldPos = mul(gWorld, float4(vin.position, 1.0));
	vout.svPosition = mul(gViewProj, worldPos);
    vout.wpos		= worldPos.xyz;
    vout.wnrm		= vin.normal;
	return vout;
}

float4 PS(VertexOut pin) : SV_Target {
	Material mat = {
		gDiffuseAlbedo,
		gFresnelR0,
		1.0 - gRoughness,
		gMetallic,
	};
	
	float3 V = normalize(gEyePos - pin.wpos);
	float3 N = normalize(pin.wnrm);
    float3 shadowFactor[16];
    shadowFactor[0] = float3(1, 1, 1);
    float3 result = ComputeLighting(gLights, mat, pin.wpos, N, V, shadowFactor);
	result += gAmbientLight * gDiffuseAlbedo;
	return float4(result, gDiffuseAlbedo.a);
}