#include "../../Component/D3D/shader/ShaderCommon.hlsl"
#include "../../Component/D3D/shader/LightingUtil.hlsl"

cbuffer CObjectCB : register(b0) {
    float4x4 gMatWorld;
    float4x4 gMatNormal;
    Material gMaterial;
};

cbuffer CPassCB : register(b1) {
    PassCBType gPass;
};

cbuffer CLight : register(b2) {
    LightCBType gLight;
};

struct VertexIn {
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 texcoord : TEXCOORD;
};

struct VertexOut {
    float4 SVPosition : SV_Position;
    float3 position   : POSITION;
    float3 normal     : NORMAL;
    float2 texcoord   : TEXCOORD;
};

VertexOut VS(VertexIn vin) {
    VertexOut vout;
    float4 worldPosition = mul(gMatWorld, float4(vin.position, 1.0));
    vout.SVPosition = mul(gPass.viewProj, worldPosition);
    vout.position   = worldPosition.xyz;
    vout.normal     = mul(gMatNormal, float4(vin.normal, 0.0)).xyz;
    vout.texcoord   = vin.texcoord;
    return vout;
}

Texture2D gDiffuseMap : register(t0);
float4 PS(VertexOut pin) : SV_Target {
    float4 diffuseAlbedo = gDiffuseMap.Sample(gSamAnisotropicWrap, pin.texcoord);
    Material mat = { diffuseAlbedo, gMaterial.roughness, gMaterial.metallic, 0, 0 };
    float3 result = { 0, 0, 0 };
    float3 viewDir = gPass.eyePos - pin.position;
    result += ComputeDirectionLight(gLight.lights[0], mat, pin.normal, viewDir);
    result += diffuseAlbedo.rgb * gLight.ambientLight.rgb;
    return float4(result, diffuseAlbedo.a);
}