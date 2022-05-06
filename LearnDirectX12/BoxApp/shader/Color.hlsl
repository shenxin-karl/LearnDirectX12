#include "../../Component/D3D/HlslShader/ShaderCommon.hlsl"

struct VertexIn {
	float3 position : POSITION;
    float3 normal   : NORMAL;
};

struct VertexOut {
    float4 position : SV_Position;
    float3 normal   : NORMAL;
};

cbuffer CBObject : register(b0) {
    float4x4 gMatWorldViewProj;
    float4x4 gMatNormal;
    Material gMaterail;
};

VertexOut VS(VertexIn vin) {
    VertexOut vout;
    vout.position = mul(gMatWorldViewProj, float4(vin.position, 1.0));
    vout.normal = mul((float3x3)gMatNormal, vin.normal);
    return vout;
}

float4 PS(VertexOut pin) : SV_Target {
    return float4(pin.normal * 0.5 + 0.5, 1.0);
}