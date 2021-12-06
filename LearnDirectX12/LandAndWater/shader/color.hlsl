#include "../../Component/D3D/shader/PassConstantBuffer.hlsl"

struct VertexIn {
    float3 position : POSITIONT;
    float4 color    : COLOR;
};

struct VertexOut {
    float4 SVPosition : SV_Position;
    float4 color      : COLOR;
};

cbuffer ObjCBBuffer : register(b1) {
    float4x4 gWorld;
};

VertexOut VS(VertexIn vin) {
    VertexOut vout;
    float4 wpos     = mul(gWorld, float4(vin.position, 1.0));
    vout.SVPosition = mul(gViewProj, wpos);
    vout.color      = vin.color;
    return vout;    
}

float4 PS(VertexOut pin) : SV_Target {
    return pin.color;
}