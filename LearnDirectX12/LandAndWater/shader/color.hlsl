#include "../../Component/D3D/shader/CBRegisterType.hlsl"
#include "../../Component/D3D/shader/LightingUtil.hlsl"
#include "../../Component/D3D/shader/PassConstantBuffer.hlsl"
#include "../../Component/D3D/shader/MaterialConstantBuffer.hlsl"

struct VertexIn {
    float3 position : POSITION;
    float4 color    : COLOR;
    float3 normal   : NORMAL;
};

struct VertexOut {
    float4 SVPosition : SV_Position;
    float3 wpos       : POSITIONT;
    float4 color      : COLOR;
    float3 wnrm       : NORMAL;
};

cbuffer ObjCBBuffer : register(CB_OBJECT_TYPE) {
    float4x4 gWorld;
};

VertexOut VS(VertexIn vin) {
    VertexOut vout;
    float4 wpos     = mul(gWorld, float4(vin.position, 1.0));
    vout.SVPosition = mul(gViewProj, wpos);
    vout.color      = vin.color;
    vout.wpos       = wpos;
    vout.wnrm       = vin.normal;
    return vout;    
}

float4 PS(VertexOut pin) : SV_Target {
    Material material = { 
        gDiffuseAlbedo,
        gFresnelR0,
        1.0f - gRoughness,
        gMetallic
    };
    float3 N = normalize(pin.wnrm);
    float3 V = normalize(gEyePos - pin.wpos);
    float3 shadowFactor[16];
    shadowFactor[0] = { 0, 0, 0 };
    float3 result = ComputeLighting(gLights, mat, N, V, shadowFactor);
    return float4(result, gDiffuseAlbedo.a);
}