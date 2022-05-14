#include "../../Component/D3D/HlslShader/ShaderCommon.hlsl"
#include "../../Component/D3D/HlslShader/LightingUtil.hlsl"

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
    Material gMaterial;
    SH3      gSH3;
};

VertexOut VS(VertexIn vin) {
    VertexOut vout;
    vout.position = mul(gMatWorldViewProj, float4(vin.position, 1.0));
    vout.normal = mul((float3x3)gMatNormal, vin.normal);
    return vout;
}

TextureCube gEnvMap : register(t0);
float4 PS(VertexOut pin) : SV_Target {
    float3 envColor = SampleSH(gSH3, normalize(pin.normal));
    return float4(envColor, 1.0);
}