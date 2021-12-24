#include "../../Component/D3D/shader/CommonDefines.hlsl"
#include "../../Component/D3D/shader/LightingUtil.hlsl"
#include "../../Component/D3D/shader/PassConstantBuffer.hlsl"
#include "../../Component/D3D/shader/MaterialConstantBuffer.hlsl"

struct VertexIn {
    float3  position : POSITION;
    float3  normal   : NORMAL;
    float2  texcoord : TEXCOORD;
};

struct VertexOut {
    float4 SVPosition : SV_Position;
    float3 wpos       : POSITION;
    float3 wnrm       : NORMAL;
    float2 tex        : TEXCOORD;
};

VertexOut VS(VertexIn vin) {
    VertexOut vout;
    float4 wpos = mul(gWorldMat, float4(vin.position, 1.0f));
    float3 wnrm = mul((float3x3)gNormalMat, vin.normal);
    float4 tex  = mul(gTextureMat, mul(gMatTransfrom, float4(vin.texcoord, 0.f, 0.f)));
    
    vout.SVPosition = mul(gViewProj, wpos);
    vout.wpos = wpos.xyz;
    vout.wnrm = wpos.xyz;
    vout.tex  = tex.xy;
    return vout;
}

Texture2D gDiffuseMap : register(TEXTURE_DIFFUSE_TYPE);
float4 PS(VertexOut pin) : SV_Target { 
    float4 diffuseAlbedo = gDiffuseMap.Sample(gSamPointClamp, pin.tex) * gDiffuseAlbedo;
    Material mat = {
        diffuseAlbedo,
        gFresnelR0,
        1.0f - gRoughness,
        gMetallic
    };
    
    float3 N = normalize(pin.wnrm);
    float3 V = normalize(gEyePos - pin.wpos);
    float3 shadowFactor[16];
    shadowFactor[0] = float3(1.0f, 1.0f, 1.0f);
    shadowFactor[1] = float3(1.0f, 1.0f, 1.0f);
    shadowFactor[2] = float3(1.0f, 1.0f, 1.0f);
    float4 result = ComputeLighting(gLights, mat, pin.wpos, N, V, shadowFactor);
    return float4(result.xyz, gDiffuseAlbedo.a);
}