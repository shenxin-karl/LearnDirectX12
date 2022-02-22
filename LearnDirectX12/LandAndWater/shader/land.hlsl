#include "../../Component/D3D/shader/ShaderCommon.hlsl"
#include "../../Component/D3D/shader/LightingUtil.hlsl"

cbuffer CPassCB : register(b0){
    PassCBType gPass;
};

cbuffer CLightCB : register(b1) {
    LightCBType gLight;
};

cbuffer CObjectCB : register(b2) {
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
    return vout;
}

Texture2D gAlbedoMap : register(t0);
float4 PS(VertexOut pin) : SV_Target {
    return pin.wnrm * 0.5 + 0.5;
}