// #define USE_CARTOON_SHADING  
#include "../../Component/D3D/HlslShader/ShaderCommon.hlsl"
#include "../../Component/D3D/HlslShader/LightingUtil.hlsl"

cbuffer CBPass : register(b0) {
	CBPassType gPassCB;
};

cbuffer CBLight : register(b1) {
	CBLightType gLight;
};

cbuffer CBObject : register(b2) {
	float4x4     gWorld;
	float4x4     gMatNormal;
	float4x4     gMatTexCoord;
	MaterialData gMaterial;
};

struct VertexIn {
	float3 position : POSITION;
	float3 normal   : NORMAL;
};

struct VertexOut {
	float4 svPosition : SV_POSITION;
	float3 wpos       : POSITION;
	float3 wnrm       : NORMAL;
};

VertexOut VS(VertexIn vin) {
	VertexOut vout;
	float4 worldPos = mul(gWorld, float4(vin.position, 1.0f));
	vout.svPosition = mul(gPassCB.viewProj, worldPos);
	vout.wpos = worldPos;
	vout.wnrm = mul((float3x3)gMatNormal, vin.normal);
	return vout;
}

TextureCube gCubeMap : register(t0);
float3 BoxCubeMapLookup(MaterialData material, float3 N, float3 V) {
	float cosIncidenceAngle = saturate(dot(N, V));
	float3 R0 = lerp(0.04, material.diffuseAlbedo.rgb, material.metallic);
	float3 specFactor = SchlickFresnelRoughness(cosIncidenceAngle, R0, material.roughness);

	float3 R = reflect(-V, N);
	float3 spec = gCubeMap.Sample(gSamLinearWrap, R).rgb * specFactor;
	spec *= (1.0 - material.roughness);
	return spec;
}

float4 PS(VertexOut pin) : SV_Target { 
    float3 viewDir = gPassCB.eyePos - pin.wpos;
    float3 result = float3(0, 0, 0);
	float3 N = normalize(pin.wnrm);
	float3 V = normalize(viewDir);
	result += ComputeDirectionLight(gLight.lights[0], gMaterial, N, V);
    result += ComputePointLight(gLight.lights[1], gMaterial, N, V, pin.wpos);
    result += ComputeSpotLight(gLight.lights[2], gMaterial, N, V, pin.wpos);
    result += gMaterial.diffuseAlbedo.rgb * gLight.ambientLight.rgb;
    result += (gMaterial.diffuseAlbedo * gLight.ambientLight).rgb;
	result += BoxCubeMapLookup(gMaterial, N, V);
	return float4(result, gMaterial.diffuseAlbedo.a);
}