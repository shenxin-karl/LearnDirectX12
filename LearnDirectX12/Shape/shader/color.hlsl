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
	float4x4  gWorld;
	float4x4  gMatNormal;
	float4x4  gMatTexCoord;
	Material  gMaterial;
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
float3 BoxCubeMapLookup(Material material, float3 viewDir, float3 normal) {
	float cosIncidenceAngle = max(dot(normalize(viewDir), normalize(normal)), 0.0);
	float3 R0 = lerp(0.04, material.diffuseAlbedo.rgb, material.metallic);
	float3 specFactor = SchlickFresnelRoughness(cosIncidenceAngle, R0, material.roughness);

	float3 R = reflect(-viewDir, normal);
	float3 spec = gCubeMap.Sample(gSamLinearWrap, R).rgb * specFactor;
	spec *= (1.0 - material.roughness);
	return spec;
}

float4 PS(VertexOut pin) : SV_Target { 
    float3 viewDir = gPassCB.eyePos - pin.wpos;
    float3 result = float3(0, 0, 0);
	result += ComputeDirectionLight(gLight.lights[0], gMaterial, pin.wnrm, viewDir);
    result += ComputePointLight(gLight.lights[1], gMaterial, pin.wnrm, viewDir, pin.wpos);
    result += ComputeSpotLight(gLight.lights[2], gMaterial, pin.wnrm, viewDir, pin.wpos);
    result += gMaterial.diffuseAlbedo.rgb * gLight.ambientLight.rgb;
    result += (gMaterial.diffuseAlbedo * gLight.ambientLight).rgb;
	result += BoxCubeMapLookup(gMaterial, viewDir, pin.wnrm);
	return float4(result, gMaterial.diffuseAlbedo.a);
}