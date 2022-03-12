#include "../../Component/D3D/shader/ShaderCommon.hlsl"
#include "../../Component/D3D/shader/LightingUtil.hlsl"

cbuffer CPassCB : register(b0) {
    PassCBType gPass;
};

cbuffer CLightCB : register(b1){
    LightCBType gLight;
};

cbuffer CObjectCB : register(b2){
    float4x4 gWorld;
    float4x4 gNormalMat;
    float4x4 gMatTransfrom;
    Material gMaterial;
};

struct VertexIn {
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 texcoord : TEXCOORD;
};

struct VertexOut {
    float4 SVPosition : SV_Position;
    float3 wpos       : POSITION;
    float3 wnrm       : NORMAL;
    float2 texcoord   : TEXCOORD;
};

VertexOut VS(VertexIn vin) {
    VertexOut vout;
    float4 worldPosition = mul(gWorld, float4(vin.position, 1.0));
    vout.SVPosition = mul(gPass.viewProj, worldPosition);
    vout.wpos       = worldPosition.xyz;
    vout.wnrm       = mul(gNormalMat, float4(vin.normal, 1.0)).xyz;
    vout.texcoord   = mul(gMatTransfrom, float4(vin.texcoord, 0.0, 0.0)).xy;
    return vout;
}

Texture2D gDiffuseMap : register(t0);
float4 PS(VertexOut pin) : SV_Target {
    float4 diffuseAlbedo = gDiffuseMap.Sample(gSamAnisotropicClamp, pin.texcoord);
    clip(diffuseAlbedo.a - 0.1);
    Material mat = {
        diffuseAlbedo, gMaterial.roughness, gMaterial.metallic,
        0.0, 0.0
    };
    
    float3 result = float3(0.0, 0.0, 0.0);
    float3 viewDir = gPass.eyePos - pin.wpos;
    result += ComputeDirectionLight(gLight.lights[0], mat, pin.wnrm, viewDir);
    result += ComputeDirectionLight(gLight.lights[1], mat, pin.wnrm, viewDir);
    result += ComputeDirectionLight(gLight.lights[2], mat, pin.wnrm, viewDir);
    result += (diffuseAlbedo * gLight.ambientLight).rgb;
    
    float dis = distance(pin.wpos, gPass.eyePos);
    float fogFactor = CalcFogAttenuation(dis, gPass.fogStart, gPass.fogEnd);
    return lerp(float4(result, 1.0), gPass.fogColor, fogFactor);
};