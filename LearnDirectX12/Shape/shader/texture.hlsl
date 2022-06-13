#include "../../Component/D3D/HlslShader/ShaderCommon.hlsl"
#include "../../Component/D3D/HlslShader/LightingUtil.hlsl"

cbuffer CBPass : register(b0){
    CBPassType gPassCB;
};

cbuffer CBLight : register(b1){
    CBLightType gLight;
};

cbuffer CBObject : register(b2){
    float4x4 gMatWorld;
    float4x4 gMatNormal;
    float4x4 gMatTexCoord;
    Material gMaterial;
};

struct VertexIn {
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangent  : TANGENT;
};

struct VertexOut {
    float4 SVPosition : SV_Position;
    float3 position   : POSITION;
    float3 normal     : NORMAL;
    float2 texcoord   : TEXCOORD;
    float3 tangent    : TANGENT;
};

VertexOut VS(VertexIn vin) {
    VertexOut vout;
    float4 worldPos = mul(gMatWorld, float4(vin.position, 1.0));
    vout.SVPosition = mul(gPassCB.viewProj, worldPos)    ;
    vout.position   = worldPos.xyz;
    vout.normal     = mul((float3x3)gMatNormal, vin.normal);
    vout.tangent    = mul((float3x3)gMatNormal, vin.tangent);
    vout.texcoord   = mul(gMatTexCoord, float4(vin.texcoord, 0.0, 1.0)).xy;
    return vout;
}

Texture2D gAlbedoMap : register(t0);
Texture2D gNormalMap : register(t1);

float3 NormalSampleToWorldSpace(VertexOut pin) {
	float3 normal = gNormalMap.Sample(gSamLinearWrap, pin.texcoord).rgb * 2.0 - 1.0;
    float3 T = normalize(pin.tangent);
    float3 N = normalize(pin.normal);
    T = (T - dot(T, N) * N);
    float3 B = cross(N, T);
    return normal.x * T +
           normal.y * B +
           normal.z * N ;
}

float4 PS(VertexOut pin) : SV_Target{
    float3 viewDir = gPassCB.eyePos - pin.position;
    float3 result = float3(0.0, 0.0, 0.0);
       
    float4 diffAlbedo = gAlbedoMap.Sample(gSamLinearWrap, pin.texcoord) * gMaterial.diffuseAlbedo;
    Material mat = {
        diffAlbedo,
        gMaterial.roughness,
        gMaterial.metallic,
        0.0, 0.0
    };

    float3 normal = NormalSampleToWorldSpace(pin);
    result += ComputeDirectionLight(gLight.lights[0], mat, normal, viewDir);
    result += ComputePointLight(gLight.lights[1], gMaterial, normal, viewDir, pin.position);
    result += ComputeSpotLight(gLight.lights[2], gMaterial, normal, viewDir, pin.position);
    result += (diffAlbedo * gLight.ambientLight).rgb;
    //return float4(result, diffAlbedo.a);

    float3 T = normalize(pin.tangent);
    return float4(T * 0.5 + 0.5, 0.0);
}
