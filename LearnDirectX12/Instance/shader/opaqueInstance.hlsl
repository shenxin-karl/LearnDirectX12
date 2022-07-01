#include "../../Component/D3D/HlslShader/ShaderCommon.hlsl"
#include "../../Component/D3D/HlslShader/LightingUtil.hlsl"

#ifndef NUM
#define NUM 5
#endif

cbuffer CBPass : register(b0) {
	CBPassType gPass;
};

cbuffer CBLight : register(b1) {
	CBLightType gLight;
};

struct VertexIn {
	float3 position : POSITION;
	float3 normal   : NORMAL;
};

struct VertexOut {
	float4 PosH : SV_Position;
	float3 PosW : POSITION;
	float3 NrmW : NORMAL;
	nointerpolation uint materialIndex : MATERIALINDEX;
	nointerpolation uint diffuseMapIdx : DIFFUSEMAPINDEX;
};

struct InstanceData {
	float4x4 matWorld;
	float4x4 matNormal;
	uint materialIdx;
	uint diffuseMapIdx;
	uint pad0;
	uint pad1;
};

StructuredBuffer<InstanceData> gInstanceData : register(t0, space1);

VertexOut VS(VertexIn vin, uint instanceId : SV_InstanceID) {
	VertexOut vout;
	InstanceData instData = gInstanceData[instanceId];
	float4 worldPosition = mul(instData.matWorld, float4(vin.position, 1.0));
	float4 worldNormal = mul(instData.matNormal, float4(vin.normal, 0.0));

	vout.PosH = mul(gPass.viewProj, worldPosition);
	vout.PosW = worldPosition.xyz;
	vout.NrmW = worldNormal.xyz;
	vout.materialIndex = instData.materialIdx;
	vout.diffuseMapIdx = instData.diffuseMapIdx;
	return vout;
}

Texture2D gDiffuseMapArray[NUM] : register(t0);
StructuredBuffer<MaterialData> gMaterialData : register(t1, space1);

float4 TriplanarMapping(Texture2D diffuseMap, float3 worldPosition, float3 normal) {
	float3 N = abs(normalize(normal));
	float4 albedo1 = diffuseMap.Sample(gSamAnisotropicWrap, worldPosition.xy);
	float4 albedo2 = diffuseMap.Sample(gSamAnisotropicWrap, worldPosition.yz);
	float4 albedo3 = diffuseMap.Sample(gSamAnisotropicWrap, worldPosition.zx);
	return (albedo1 * N.z) + (albedo2 * N.x) + (albedo3 * N.y);
}

float4 PS(VertexOut pin) : SV_Target {
	MaterialData mat = gMaterialData[pin.materialIndex];
	float4 albedo = TriplanarMapping(gDiffuseMapArray[pin.diffuseMapIdx], pin.PosW, pin.NrmW);
	mat.diffuseAlbedo *= albedo;

	float3 viewDir = gPass.eyePos - pin.PosW;

	float3 result = float3(0.0, 0.0, 0.0);
	result += (gLight.ambientLight * mat.diffuseAlbedo).xyz;
	result += ComputeDirectionLight(gLight.lights[0], mat, pin.NrmW, viewDir);
	result += ComputeDirectionLight(gLight.lights[1], mat, pin.NrmW, viewDir);
	result += ComputeDirectionLight(gLight.lights[2], mat, pin.NrmW, viewDir);
	return float4(result, mat.diffuseAlbedo.a);
}