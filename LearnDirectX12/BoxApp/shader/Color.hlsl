#include "../../Component/D3D/HlslShader/ShaderCommon.hlsl"
#include "../../Component/D3D/HlslShader/LightingUtil.hlsl"
#include "../../Component/D3D/HlslShader/ColorGrading.hlsl"

struct VertexIn {
	float3 position : POSITION;
    float3 normal   : NORMAL;
};

struct VertexOut {
    float4 SVPosition : SV_Position;
    float3 position   : POSITION;
    float3 normal     : NORMAL;
};

cbuffer CBObject : register(b0) {
    float4x4     gMatWorld;
    float4x4     gMatNormal;
    MaterialData gMaterial;
    SH3          gSH3;
};

cbuffer CBPass : register(b1) {
    CBPassType gPass;
};

VertexOut VS(VertexIn vin) {
    VertexOut vout;
    float4 worldPosition = mul(gMatWorld, float4(vin.position, 1.0));
    vout.SVPosition = mul(gPass.viewProj, worldPosition);
    vout.position = (float3)worldPosition;
    vout.normal = mul((float3x3)gMatNormal, vin.normal);
    return vout;
}

TextureCube gEnvMap          : register(t0);
TextureCube gPerFilterEnvMap : register(t1);
Texture2D   gBRDFLut         : register(t2);

float4 PS(VertexOut pin) : SV_Target {
    IBLParam param = {
        gSH3,
        gSamLinearClamp,
        gBRDFLut,
        gPerFilterEnvMap
    };

    float3 N = normalize(pin.normal);
    float3 V = normalize(gPass.eyePos - pin.position);
    float3 envColor = CalcIBLAmbient(V, N, gMaterial, param);
    envColor = ACESToneMapping(envColor, 1.0);
    envColor = ConvertToSRGB(envColor);
    return float4(envColor, 1.0);
}