#include "ShaderCommon.hlsl"
#include "LightingUtil.hlsl"
#include "ColorGrading.hlsl"

struct VertexIn {
	float3 position;
	float2 texcoord;
	float3 normal;
    float3 tangent;
};

struct VertexOut {
	float4 SVPosition;
	float3 position;
	float2 texcoord;
	float3 normal;
	float3 tangent;
};

cbuffer CBOjbect : register(b0) {
	float4x4	 gMatWorld;
	float4x4	 gMatNormal;
	float4x4	 gMatTexCoord;
	MaterialData gMaterialData;
};

cbuffer CBPass : register(b1) {
	CBPassType gPass;
};

cbuffer CBLight : register(b2) {
	CBLightType gLight;
};

VertexOut VS(VertexIn vin) {
    VertexOut vout;
	float4 worldPosition = mul(gMatWorld, float4(vin.position, 1.0));
	vout.SVPosition = mul(gPass.viewProj, worldPosition);
	vout.position   = worldPosition.xyz;
	vout.texcoord   = mul(gMatTexCoord, float4(vin.texcoord, 0.0, 1.0)).xy;
	vout.normal     = mul((float3x3)gMatNormal, vin.normal);
	vout.tangent    = mul((float3x3)gMatWorld, vin.tangent);
	return vout;
}

Texture2D gAlbedoMap    : register(t0);
Texture2D gNormalMap    : register(t1);
Texture2D gRoughnessMap : register(t2);
Texture2D gMetallicMap  : register(t3);
Texture2D gAOMap        : register(t4);

float3 TangentSpaceToWorldSpace(VertexOut pin, float3 texNormal) {
    float3 T = normalize(pin.tangent);
    float3 N = normalize(pin.normal);
    T = (T - dot(T, N) * N);
    float3 B = cross(N, T);
    texNormal = texNormal * 2.0 - 1.0;
    return texNormal.x * T +
		   texNormal.y * B +
		   texNormal.z * N ;
}

float4 PS(VertexOut pin) : SV_Target {
    float4 textureAlbedo = float4(gAlbedoMap.Sample(gSamAnisotropicClamp, pin.texcoord).rgb, 1.0);
	float3 textureNormal = gNormalMap.Sample(gSamAnisotropicClamp, pin.texcoord).rgb;
	float textureRoughness = gRoughnessMap.Sample(gSamAnisotropicClamp, pin.texcoord).r;
	float textureMetallic = gMetallicMap.Sample(gSamAnisotropicClamp, pin.texcoord).r;
	float textureAO = gMetallicMap.Sample(gSamAnisotropicClamp, pin.texcoord).r;

	MaterialData materialData = {
        gMaterialData.diffuseAlbedo * textureAlbedo,
		gMaterialData.roughness * textureRoughness,
		gMaterialData.metallic * textureMetallic
    };

	float3 N = normalize(TangentSpaceToWorldSpace(pin, textureNormal));
	float3 V = normalize(gPass.eyePos - pin.position);
    float3 result = (textureAO * materialData.diffuseAlbedo * gLight.ambientLight).rgb;

	float shadows[kMaxLightCount] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    result += ComputeLight(gLight.lights, shadows, materialData, N, V, pin.position);

	return float4(result, materialData.diffuseAlbedo.a);
}