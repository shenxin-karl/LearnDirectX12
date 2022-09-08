#include "../../Component/D3D/HlslShader/ShaderCommon.hlsl"
#include "../../Component/D3D/HlslShader/LightingUtil.hlsl"
#include "../../Component/D3D/HlslShader/ColorGrading.hlsl"

cbuffer CbTransform : register(b0) {
	float4x4	 gMatWorld;
	float4x4	 gMatNormal;
};

cbuffer CbObject : register(b1) {
	MaterialData gMaterialData;
	float4x4	 gMatTexCoord;
};

cbuffer CbPass : register(b2) {
	CBPassType gPass;
};

cbuffer CbLight : register(b3) {
	CBLightType gLight;
};

cbuffer cbShadow : register(b4) {
	float4x4 gLightSpaceMatrix[7];
};

struct VertexIn {
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
	float3 normal   : NORMAL;
};

struct VertexOut {
	float4 SVPosition     : SV_Position;
	float3 position       : POSITION;
	float2 texcoord       : TEXCOORD;
	float3 normal         : NORMAL;
	float4 lightSpacePos0 : TEXCOORD1;
	float4 lightSpacePos1 : TEXCOORD2;
	float4 lightSpacePos2 : TEXCOORD3;
	float4 lightSpacePos3 : TEXCOORD4;
};

VertexOut VS(VertexIn vin) {
	VertexOut vout;
	float4 worldPosition = mul(gMatWorld, float4(vin.position, 1.0));
	vout.SVPosition = mul(gPass.viewProj, worldPosition);
	vout.position = worldPosition.xyz;
	vout.texcoord = mul(gMatTexCoord, float4(vin.texcoord, 0.0, 1.0)).xy;
	vout.normal = mul((float3x3)gMatNormal, vin.normal);

	vout.lightSpacePos0 = mul(gLightSpaceMatrix[0], worldPosition);
	vout.lightSpacePos1 = mul(gLightSpaceMatrix[1], worldPosition);
	vout.lightSpacePos2 = mul(gLightSpaceMatrix[2], worldPosition);
	vout.lightSpacePos3 = mul(gLightSpaceMatrix[3], worldPosition);
	return vout;
}

float4 getShadowColor(VertexOut pin) {
	float4 lightSpacePos[] = {
		pin.lightSpacePos0,
		pin.lightSpacePos1,
		pin.lightSpacePos2,
		pin.lightSpacePos3
	};

	float4 colorList[] = {
		float4(1.0, 0.0, 0.0, 1.0),
		float4(0.0, 1.0, 0.0, 1.0),
		float4(0.3, 0.0, 0.5, 1.0),
		float4(0.0, 0.0, 1.0, 1.0),
		float4(0.0, 1.0, 1.0, 1.0),
		float4(1.0, 0.0, 1.0, 1.0),
		float4(1.0, 1.0, 1.0, 1.0),
	};

	for (int i = 0; i < 4; ++i) {
		float4 pos = lightSpacePos[i];
		pos.xyz /= pos.w;
		if (pos.x >= 0.00 && pos.x <= 1.0 && pos.y >= 0.00 && pos.y <= 1.0)
			return colorList[i];
	}
	return float4(0.3, 0.3, 0.3, 1.0);
}

Texture2D gAlbedoMap : register(t0);
float4 PS(VertexOut pin) : SV_Target{
	float4 textureAlbedo = gAlbedoMap.Sample(gSamLinearWrap, pin.texcoord);
	textureAlbedo *= getShadowColor(pin);
	MaterialData materialData = {
		gMaterialData.diffuseAlbedo * textureAlbedo,
		gMaterialData.roughness,
		gMaterialData.metallic,
		0.0, 0.0,
	};

	float3 viewDir = gPass.eyePos - pin.position;
	float3 result = 0.0;
	float3 V = normalize(viewDir);
	float3 N = normalize(pin.normal);
	result += ComputeDirectionLight(gLight.lights[0], materialData, N, V);
	result += ComputeDirectionLight(gLight.lights[1], materialData, N, V);
	result += ComputeDirectionLight(gLight.lights[2], materialData, N, V);
	result += (gLight.ambientLight * textureAlbedo).rgb;
	result = GammaCorrection(result);
	return float4(result, gMaterialData.diffuseAlbedo.a);
}

