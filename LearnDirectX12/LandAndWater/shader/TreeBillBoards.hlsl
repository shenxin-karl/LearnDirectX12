#include "../../Component/D3D/HlslShader/ShaderCommon.hlsl"
#include "../../Component/D3D/HlslShader/LightingUtil.hlsl"

cbuffer CPassCB : register(b0) {
    CBPassType gPass;
};

cbuffer CLightCB : register(b1){
    CBLightType gLight;
};

cbuffer COjbectCB : register(b2){
    float4x4     gMatWorld;
    float4x4     gMatNormal;
    float4x4     gMatTransform;
    MaterialData gMaterial;
};

struct VertexIn {
    float3 position : POSITION;
    float2 size     : SIZE;
};

struct VertexOut {
    float3 position   : POSITION;
    float2 size       : SIZE;
};

VertexOut VS(VertexIn vin) {
    VertexOut vout;
    float4 worldPosition = mul(gMatWorld, float4(vin.position, 1.0));
    vout.position = worldPosition.xyz;
    vout.size = vin.size;
    return vout;
}

struct GeometryOut {
    float4 SVPosition : SV_Position;
    float3 position   : POSITION;
    float3 normal     : NORMAL;
    float2 texcoord   : TEXCOORD;
    uint   primID     : SV_PrimitiveID;
};

[maxvertexcount(4)]
void GS(point VertexOut gin[1], uint primID : SV_PrimitiveID, inout TriangleStream<GeometryOut> triStream) {
    float3 v = float3(0.0, 1.0, 0.0);
    float3 w = gPass.eyePos - gin[0].position;
    w.y = 0.0;
    w = normalize(w);
    float3 u = cross(v, w);
    float3 halfU = gin[0].size.x * 0.5 * u;
    float3 halfV = gin[0].size.y * 0.5 * v;
    
    float3 center = gin[0].position;
    float3 vertices[4] = {
        center + halfU - halfV,
        center + halfU + halfV,
        center - halfU - halfV,
        center - halfU + halfV,
    };
    
    const float2 texcoords[4] = {
        float2(0.0, 1.0),
        float2(0.0, 0.0),
        float2(1.0, 1.0),
        float2(1.0, 0.0),
    };

    [unroll(4)]
    for (int i = 0; i < 4; ++i) {
        GeometryOut vert;
        vert.SVPosition = mul(gPass.viewProj, float4(vertices[i], 1.0));
        vert.position = vertices[i];
        vert.normal   = w;
        vert.texcoord = texcoords[i];
        vert.primID   = primID;
        triStream.Append(vert);
    }
}

Texture2DArray gDiffuseMap : register(t0);
float4 PS(GeometryOut pin) : SV_Target {
    //return float4(1, 0, 0, 1);
    float3 texcoord = float3(pin.texcoord, pin.primID % 3);
    float4 diffuseAlbedo = gDiffuseMap.Sample(gSamAnisotropicClamp, texcoord);
    diffuseAlbedo *= gMaterial.diffuseAlbedo;
    clip(diffuseAlbedo.a - 0.1);
    float3 result = float3(0.0, 0.0, 0.0);
    float3 viewDir = gPass.eyePos - pin.position;
    MaterialData mat = { diffuseAlbedo, gMaterial.roughness, gMaterial.metallic, 0.0, 0.0 };
    result += ComputeDirectionLight(gLight.lights[0], mat, pin.normal, viewDir);
    result += ComputeDirectionLight(gLight.lights[1], mat, pin.normal, viewDir);
    result += ComputeDirectionLight(gLight.lights[2], mat, pin.normal, viewDir);
    result += (gLight.ambientLight * diffuseAlbedo).xyz;
    
    // FOG
    float dis = distance(pin.position, gPass.eyePos);
    float fogFactor = CalcFogAttenuation(dis, gPass.fogStart, gPass.fogEnd);
    return lerp(float4(result, 1.0), gPass.fogColor, fogFactor);

}