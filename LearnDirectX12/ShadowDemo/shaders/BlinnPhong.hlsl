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
	float4x4 gWorldToShadowMatrix[7];
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

	vout.lightSpacePos0 = mul(gWorldToShadowMatrix[0], worldPosition);
	vout.lightSpacePos1 = mul(gWorldToShadowMatrix[1], worldPosition);
	vout.lightSpacePos2 = mul(gWorldToShadowMatrix[2], worldPosition);
	vout.lightSpacePos3 = mul(gWorldToShadowMatrix[3], worldPosition);

	return vout;
}

static const float kShadowSampleKernel = 3;
static const float kBlockerSearchWidth = 5;
static const float kMaxSampleKernel = max(kShadowSampleKernel, kBlockerSearchWidth);

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


static float2 poissonDisk[16] = { 
   float2( -0.94201624, -0.39906216 ), 
   float2( 0.94558609, -0.76890725 ), 
   float2( -0.094184101, -0.92938870 ), 
   float2( 0.34495938, 0.29387760 ), 
   float2( -0.91588581, 0.45771432 ), 
   float2( -0.81544232, -0.87912464 ), 
   float2( -0.38277543, 0.27676845 ), 
   float2( 0.97484398, 0.75648379 ), 
   float2( 0.44323325, -0.97511554 ), 
   float2( 0.53742981, -0.47373420 ), 
   float2( -0.26496911, -0.41893023 ), 
   float2( 0.79197514, 0.19090188 ), 
   float2( -0.24188840, 0.99706507 ), 
   float2( -0.81409955, 0.91437590 ), 
   float2( 0.19984126, 0.78641367 ), 
   float2( 0.14383161, -0.14100790 ) 
};

// Returns a random number based on a float3 and an int.
float random(float3 seed, int i){
	float4 seed4 = float4(seed, i);
	float dot_product = dot(seed4, float4(12.9898,78.233,45.164,94.673));
	return frac(sin(dot_product) * 43758.5453);
}

float2 AverageBlockerDepth(float3 lightSpacePos, int csmIndex, float searchWidth) {
	uint width, height, planeSlice;
	gShadowMapArray.GetDimensions(width, height, planeSlice);
	float dx = 1.0 / (float)width;

	float step = (int)(kBlockerSearchWidth / 2);
	float average = 0.0;
	float count = 0.0005;							// ·ÀÖ¹³ý 0
	float range = step / searchWidth * dx;
	float3 basePos = float3(lightSpacePos.xy, csmIndex);
	for (float i = -step; i <= step; i += 1.0) {
		for (float j = -step; j <= step; j += 1.0) {
			float2 offset = float2(i, j) * range;
			float3 samplePos = basePos;
			samplePos.xy += offset;
			float texDepth = gShadowMapArray.SampleLevel(gSamPointClamp, samplePos, 0).r;
			if (texDepth < lightSpacePos.z) {
				count += 1.0;
				average += texDepth;
			}
		}
	}
	return float2(average / count, count);
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
	float dx = 1.0 / (float)width;
	float validBegin = dx * kMaxSampleKernel;
	float validEnd = 1.f - validBegin;

	float3 pos = 0;
	int index = 0;
	while (index < 4) {
		pos = lightSpacePos[index].xyz / lightSpacePos[index].w;
		if (pos.x > validBegin && pos.x < validEnd && pos.y > validBegin && pos.y < validEnd)
			break;
		++index;
	}

	float2 ret =  AverageBlockerDepth(pos, index, 7.0);
	return 1.0 - (ret.y / (kBlockerSearchWidth * 2 + 1));

#if 0
	return pos.z <= gShadowMapArray.SampleLevel(gSamPointClamp, float3(pos.xy, index), 0).r;
#endif

	const float2 offsets[9] = {
		float2(-dx, -dx), float2(0.0, -dx), float2(+dx, -dx),
		float2(-dx, 0.0), float2(0.0, 0.0), float2(+dx, 0.0),
		float2(-dx, +dx), float2(0.0, +dx), float2(+dx, +dx),
	};

	float depth = pos.z;
	float percentLit = 0.0;
	float range = kShadowSampleKernel * dx;
	[unroll]
	for (int i = 0; i < 16; ++i) {
		float3 samplePos = float3(pos.xy, index);
		int diskIndex = int(16.0  * random(floor(pin.position.xyz * 1000.0), i)) % 16;
		samplePos.xy += poissonDisk[i] * range;
		percentLit += gShadowMapArray.SampleCmpLevelZero(gSamLinearShadowCompare, samplePos, depth).r;
	}
	percentLit /= 16.0;
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
	return shadow;

	result += ComputeDirectionLight(gLight.lights[0], materialData, N, V) * shadow;
	result += ComputeDirectionLight(gLight.lights[1], materialData, N, V);
	result += ComputeDirectionLight(gLight.lights[2], materialData, N, V);
	result += (gLight.ambientLight * textureAlbedo).rgb;
	result = GammaCorrection(result);
	return float4(result, gMaterialData.diffuseAlbedo.a);
}

