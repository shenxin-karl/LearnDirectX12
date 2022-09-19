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

#define kShadowSampleKernel 3
#define kBlockerSearchWidth 3
#define kMinSampleCount     4
#define kMaxSampleCount		32

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
	};

	uint width, height, planeSlice;
	gShadowMapArray.GetDimensions(width, height, planeSlice);
	float dx = 1.0 / (float)width;

	for (int i = 0; i < 4; ++i) {
		float3 pos = lightSpacePos[i].xyz;
		if (pos.x >= dx && pos.x <= (1.f-dx) && pos.y >= dx && pos.y <= (1.f-dx))
			return colorList[i];
	}
	return float4(0.3, 0.3, 0.3, 1.0);
}

float2 AverageBlockerDepth(float3 lightSpacePos, int csmIndex, float searchWidth) {
	uint width, height, planeSlice;
	gShadowMapArray.GetDimensions(width, height, planeSlice);
	float dx = 1.0 / (float)width;

	float step = (int)(searchWidth / 2);
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

static const float2 Poisson32[32] = {
    float2(-0.975402, -0.0711386),    float2(0.0116886, 0.326395),
    float2(-0.920347, -0.41142),      float2(0.0380566, 0.625477),
    float2(-0.883908, 0.217872),      float2(0.0625935, -0.50853),
    float2(-0.884518, 0.568041),      float2(0.125584, 0.0469069),
    float2(-0.811945, 0.90521),       float2(0.169469, -0.997253),
    float2(-0.792474, -0.779962),     float2(0.320597, 0.291055),
    float2(-0.614856, 0.386578),      float2(0.359172, -0.633717),
    float2(-0.580859, -0.208777),     float2(0.435713, -0.250832),
    float2(-0.53795, 0.716666),       float2(0.507797, -0.916562),
    float2(-0.515427, 0.0899991),     float2(0.545763, 0.730216),
    float2(-0.454634, -0.707938),     float2(0.56859, 0.11655),
    float2(-0.420942, 0.991272),      float2(0.743156, -0.505173),
    float2(-0.261147, 0.588488),      float2(0.736442, -0.189734),
    float2(-0.211219, 0.114841),      float2(0.843562, 0.357036),
    float2(-0.146336, -0.259194),     float2(0.865413, 0.763726),
    float2(-0.139439, -0.888668),     float2(0.872005, -0.927)
};

// Returns a random number based on a vec3 and an int.
float random(float3 seed, int i){
	float4 seed4 = float4(seed,i);
	float dot_product = dot(seed4, float4(12.9898,78.233,45.164,94.673));
	return frac(sin(dot_product) * 43758.5453);
}

float HardShadow(float3 lightSpacePos, int csmIndex) {
	return gShadowMapArray.SampleCmpLevelZero(gSamLinearShadowCompare, float3(lightSpacePos.xy, csmIndex), lightSpacePos.z).r;
}

float PCSS(float3 worldPosition, float3 lightSpacePos, int csmIndex, float dx) {
	float2 blockerSearchRes = AverageBlockerDepth(lightSpacePos, csmIndex, kBlockerSearchWidth);
	if (blockerSearchRes.y <= 0.1)		// 没有遮挡物, 不会产生阴影
		return 1.0;

	float near = gShadow.subFrustum[csmIndex].zNear;
	float far = gShadow.subFrustum[csmIndex].zFar;
	float3 center = gShadow.subFrustum[csmIndex].center;
	float lightPlane = gShadow.subFrustum[csmIndex].lightPlane;
	float3 lightDir = gShadow.lightDir;

	float blockerDistance = lerp(near, far, blockerSearchRes.x);
	blockerDistance *= lightDir.y;
	blockerDistance += center.y;
	blockerDistance = max(lightPlane - blockerDistance, 0.01);

	float receiverDistance = lerp(near, far, lightSpacePos.z);
	receiverDistance *= lightDir.y;
	receiverDistance += center.y;
	receiverDistance = max(lightPlane - receiverDistance, 0.001);

	float penumbraWidth = saturate((receiverDistance - blockerDistance) / blockerDistance);
	float pcfKernel = clamp(penumbraWidth * gShadow.lightSize, 1, kShadowSampleKernel) * dx;
	float sampleCount = floor(lerp(kMinSampleCount, kMaxSampleCount, penumbraWidth));

	float depth = lightSpacePos.z;
	float percentLit = 0.0;
	float3 floorWorldPosition = floor(worldPosition * 100.0);
	for (int i = 0; i < sampleCount; ++i) {
		#if defined(SHADOW_PCSS_POISSON)
			int index = int(32.0 * random(floorWorldPosition, i)) % 32;
			float2 offset = Poisson32[index] * pcfKernel;
		#else
			float2 offset = (Hammersley(i, sampleCount) * 2.0 - 1.0) * pcfKernel;
		#endif
		float3 samplePos = float3(lightSpacePos.xy + offset, csmIndex);

		percentLit += gShadowMapArray.SampleCmpLevelZero(gSamPointShadowCompare, samplePos, depth).r;
	}
	percentLit /= sampleCount;
	return percentLit;
}

float PCF(float3 lightSpacePos, int csmIndex, float dx) {
		const float2 offsets[9] = {
		float2(-dx, -dx), float2(0.0, -dx), float2(+dx, -dx),
		float2(-dx, 0.0), float2(0.0, 0.0), float2(+dx, 0.0),
		float2(-dx, +dx), float2(0.0, +dx), float2(+dx, +dx),
	};

	float depth = lightSpacePos.z;
	float percentLit = 0.0;
	for (int i = 0; i < 9; ++i) {
		float3 samplePos = float3(lightSpacePos.xy, csmIndex);
		samplePos.xy += offsets[i];
		percentLit += gShadowMapArray.SampleCmpLevelZero(gSamLinearShadowCompare, samplePos, depth).r;
	}
	percentLit /= 9.0;
	return percentLit;
}


#define SHADOW_PCSS
float getShadow(VertexOut pin) {
	float4 lightSpacePos[] = {
		pin.lightSpacePos0,
		pin.lightSpacePos1,
		pin.lightSpacePos2,
		pin.lightSpacePos3
	};

	uint width, height, planeSlice;
	gShadowMapArray.GetDimensions(width, height, planeSlice);
	#if defined(SHADOW_PCSS)
		const float kMaxSampleKernel = max(kShadowSampleKernel, kBlockerSearchWidth);
	#else
		const float kMaxSampleKernel = 1;
	#endif

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
	
	#if defined(SHADOW_PCSS)
		return PCSS(pin.position, pos, index, dx);
	#elif defined(SHADOW_PCF)
		return PCF(pos, index, dx);
	#else
		return HardShadow(pos, index);
	#endif
}


Texture2D gAlbedoMap : register(t0);
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

	result += ComputeDirectionLight(gLight.lights[0], materialData, N, V) * shadow;
	result += ComputeDirectionLight(gLight.lights[1], materialData, N, V);
	result += ComputeDirectionLight(gLight.lights[2], materialData, N, V);
	result += (gLight.ambientLight * textureAlbedo).rgb;
	result = GammaCorrection(result);
	return float4(result, gMaterialData.diffuseAlbedo.a);
}

