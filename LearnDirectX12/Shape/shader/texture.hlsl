#include "../../Component/D3D/shader/ShaderCommon.hlsl"
#include "../../Component/D3D/shader/LightingUtil.hlsl"

cbuffer CBPass : register(b0){
    CBPassType gPassCB;
};

cbuffer CBLight : register(b1){
    CBLightType gLight;
};

cbuffer CBObject : register(b2){
    float4x4 gWorld;
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
    float4 worldPositon = mul(gWorld, float4(vin.position, 1.0));
    vout.SVPosition     = mul(gPassCB.viewProj, worldPositon);
    vout.wpos           = worldPositon.xyz;
    vout.wnrm           = vin.normal;
    vout.texcoord       = vin.texcoord;
    return vout;
}

Texture2D gAlbedoMap : register(t0);
float4 PS(VertexOut pin) : SV_Target{
    float3 viewDir = gPassCB.eyePos - pin.wpos;
    float3 result = float3(0.0, 0.0, 0.0);
       
    float4 diffAlbedo = gAlbedoMap.Sample(gSamPointClamp, pin.texcoord) * gMaterial.diffuseAlbedo;
    Material mat = {
        diffAlbedo,
        gMaterial.roughness,
        gMaterial.metallic,
        0.0, 0.0
    };
    
    result += ComputeDirectionLight(gLight.lights[0], mat, pin.wnrm, viewDir);
    result += (diffAlbedo * gLight.ambientLight).rgb;
    return float4(result, diffAlbedo.a);
}
