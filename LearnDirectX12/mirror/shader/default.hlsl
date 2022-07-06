#include "../../Component/D3D/HlslShader/ShaderCommon.hlsl"
#include "../../Component/D3D/HlslShader/LightingUtil.hlsl"

cbuffer CObjectCB : register(b0) {
    float4x4     gMatWorld;
    float4x4     gMatNormal;
    MaterialData gMaterial;
};

cbuffer CPassCB : register(b1) {
    CBPassType gPass;
};

cbuffer CLight : register(b2) {
    CBLightType gLight;
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
    MaterialData mat = {
        diffuseAlbedo * gMaterial.diffuseAlbedo, 
        gMaterial.roughness, 
        gMaterial.metallic, 0, 0 
    };
    float3 result = { 0, 0, 0 };
    float3 viewDir = gPass.eyePos - pin.position;
    float3 N = normalize(pin.normal);
    float3 V = normalize(viewDir);
    result += ComputeDirectionLight(gLight.lights[0], mat, N, V);
    result += ComputeDirectionLight(gLight.lights[1], mat, N, V);
    result += ComputeDirectionLight(gLight.lights[2], mat, N, V);
    result += diffuseAlbedo.rgb * gLight.ambientLight.rgb;
    float alpha = diffuseAlbedo.a * gMaterial.diffuseAlbedo.a;
    return float4(result, alpha);
}