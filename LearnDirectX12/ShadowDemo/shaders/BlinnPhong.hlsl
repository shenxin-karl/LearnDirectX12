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
	cbShadowType gShadow;
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

	vout.lightSpacePos0 = mul(gShadow.subFrustum[0].worldToLightMatrix, worldPosition);
	vout.lightSpacePos1 = mul(gShadow.subFrustum[1].worldToLightMatrix, worldPosition);
	vout.lightSpacePos2 = mul(gShadow.subFrustum[2].worldToLightMatrix, worldPosition);
	vout.lightSpacePos3 = mul(gShadow.subFrustum[3].worldToLightMatrix, worldPosition);
	return vout;
}

static const float kShadowSampleKernel = 5;
static const float kBlockerSearchWidth = 5;
static const uint  kPCFSampleCount     = 16;

Texture2DArray gShadowMapArray : register(t1);
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
		float4(0.0, 0.0, 1.0, 1.0),
		float4(1.0, 0.0, 1.0, 1.0),
		float4(0.0, 1.0, 1.0, 1.0),
		float4(1.0, 0.1, 1.0, 1.0),
		float4(1.0, 1.0, 1.0, 1.0),
	};

	uint width, height, planeSlice;
	gShadowMapArray.GetDimensions(width, height, planeSlice);
	float dx = 1.0 / (float)width;

	for (int i = 0; i < 4; ++i) {
		float3 pos = lightSpacePos[i].xyz / lightSpacePos[i].w;
		if (pos.x >= dx && pos.x <= (1.f-dx) && pos.y >= dx && pos.y <= (1.f-dx))
			return colorList[i];
	}
	return float4(0.3, 0.3, 0.3, 1.0);
}

float2 AverageBlockerDepth(float3 lightSpacePos, int csmIndex, float searchWidth) {
	uint width, height, planeSlice;
	gShadowMapArray.GetDimensions(width, height, planeSlice);
	float dx = 1.0 / (float)width;

	float step = (int)(kBlockerSearchWidth / 2);
	float average = 0.0;
	float count = 0.0;							// 防止除 0
	float3 basePos = float3(lightSpacePos.xy, csmIndex);
	for (float i = -step; i <= step; i += 1.0) {
		for (float j = -step; j <= step; j += 1.0) {
			float2 offset = float2(i, j) / step;
			offset *= searchWidth;
			offset *= dx;
			float3 samplePos = basePos;
			samplePos.xy += offset;
			float texDepth = gShadowMapArray.SampleLevel(gSamPointClamp, samplePos, 0).r;
			if (texDepth < lightSpacePos.z) {
				count += 1.0;
				average += texDepth;
			}
		}
	}

	return count != 0.0 ? float2(average / count, count) : float2(0.0, 0.0);
}

float RadicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N) {
	return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

Texture2D gAlbedoMap		   : register(t0);
float getShadow(VertexOut pin) {
		float4 lightSpacePos[] = {
		pin.lightSpacePos0,
		pin.lightSpacePos1,
		pin.lightSpacePos2,
		pin.lightSpacePos3
	};

	uint width, height, planeSlice;
	gShadowMapArray.GetDimensions(width, height, planeSlice);
	const float kMaxSampleKernel = max(kShadowSampleKernel, kBlockerSearchWidth);

	float dx = 1.0 / (float)width;
	float validBegin = dx * kMaxSampleKernel;
	float validEnd = 1.f - validBegin;

	float3 pos = 0;
	int index = 0;
	while (index < 4) {
		pos = lightSpacePos[index].xyz;
		if (pos.x > validBegin && pos.x < validEnd && pos.y > validBegin && pos.y < validEnd)
			break;
		++index;
	}

#if 0
	return pos.z <= gShadowMapArray.SampleLevel(gSamPointClamp, float3(pos.xy, index), 0).r;
#endif

#if 1

	float2 blockerSearchRes = AverageBlockerDepth(pos, index, 3.0);
	if (blockerSearchRes.y <= 0.1)		// 没有遮挡物, 不会产生阴影
		return 1.0;

	float near = gShadow.subFrustum[index].zNear;
	float far = gShadow.subFrustum[index].zFar;
	float3 center = gShadow.subFrustum[index].center;
	float lightPlane = gShadow.subFrustum[index].lightPlane;
	float3 lightDir = gShadow.lightDir;

	float blockerDistance = lerp(near, far, blockerSearchRes.x);
	blockerDistance *= lightDir.y;
	blockerDistance += center.y;
	blockerDistance -= lightPlane;

	float receiverDistance = lerp(near, far, pos.z);
	receiverDistance *= lightDir.y;
	receiverDistance += center.y;
	receiverDistance -= lightPlane;

	float penumbraWidth = saturate((receiverDistance - blockerDistance) / 10);
	float pcfKernel = clamp(penumbraWidth * gShadow.lightSize, 1, kShadowSampleKernel) * dx;
#else
	float penumbraWidth = 2 * dx;
#endif

	float depth = pos.z;
	float percentLit = 0.0;
	for (int i = 0; i < kPCFSampleCount; ++i) {
		float2 offset = (Hammersley(i, kPCFSampleCount) * 2.0 - 1.0) * pcfKernel;
		float3 samplePos = float3(pos.xy + offset, index);
		percentLit += gShadowMapArray.SampleCmpLevelZero(gSamLinearShadowCompare, samplePos, depth).r;
	}
	percentLit /= (float)kPCFSampleCount;
	return percentLit;
}


float4 PS(VertexOut pin) : SV_Target{
	float4 textureAlbedo = gAlbedoMap.Sample(gSamLinearWrap, pin.texcoord);
	//textureAlbedo *= getShadowColor(pin);
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

	float shadow = getShadow(pin);
	 //return shadow;


	result += ComputeDirectionLight(gLight.lights[0], materialData, N, V) * shadow;
	result += ComputeDirectionLight(gLight.lights[1], materialData, N, V);
	result += ComputeDirectionLight(gLight.lights[2], materialData, N, V);
	result += (gLight.ambientLight * textureAlbedo).rgb;
	result = GammaCorrection(result);
	return float4(result, gMaterialData.diffuseAlbedo.a);
}

