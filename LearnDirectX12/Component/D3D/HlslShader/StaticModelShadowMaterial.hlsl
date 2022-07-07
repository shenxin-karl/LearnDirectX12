#include "ShaderCommon.hlsl"

struct VertexIn {
	float3 position;
	float2 texcoord;
	float3 normal;
    float3 tangent;
};

struct VertexOut {
	float4 SVPosition;
};

cbuffer CBOjbect : register(b0) {
	float4x4	 gMatWorld;
	float4x4	 gMatNormal;
	float4x4	 gMatTexCoord;
	MaterialData gMaterialData;
};

cbuffer CBLightSpaceMatrix : register(b1) {
	float4x4    gLightSpaceMatrix;
};

VertexOut VS(VertexIn vin) {
    VertexOut vout;
	float4 worldPosition = mul(gMatWorld, float4(vin.position, 1.0));
	vout.SVPosition = mul(gLightSpaceMatrix, worldPosition);
	return vout;
}

void PS(VertexOut pin) {
   
}