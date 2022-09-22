#line 2 "PerFilterEnvMapCS.hlsl"
cbuffer CBSettgings : register(b0) {
	float fSize;
    float fRoughness;
    uint  gIndex;
}

struct ComputeIn {
    uint3 	GroupID 		  : SV_GroupID;				// 线程组 ID
    uint3   GroupThreadID	  : SV_GroupThreadID;		// 组内线程 ID
    uint3   DispatchThreadID  : SV_DispatchThreadID;	// 线程 ID
    uint  	GroupIndex        : SV_GroupIndex;			// 线程组内索引
};

const static float3x3 gRotateCubeFace[6] = {
    float3x3(float3(+0,  +0, +1), float3(+0, -1, +0), float3(-1, +0, +0) ),   // +X
    float3x3(float3(+0,  +0, -1), float3(+0, -1, +0), float3(+1, +0, +0) ),   // -X
    float3x3(float3(+1,  +0, +0), float3(+0, +0, +1), float3(+0, +1, +0) ),   // +Y
    float3x3(float3(+1,  +0, +0), float3(+0, +0, -1), float3(+0, -1, +0) ),   // -Y
    float3x3(float3(+1,  +0, +0), float3(+0, -1, +0), float3(+0, +0, +1) ),   // +Z
    float3x3(float3(-1,  +0, +0), float3(+0, -1, +0), float3(+0, +0, -1) )    // -Z
};

float3 CalcDirection(ComputeIn cin) {
	uint index = gIndex;
    float x = (cin.DispatchThreadID.x / fSize) - 0.5;
    float y = (cin.DispatchThreadID.y / fSize) - 0.5;
    float3 direction = normalize(mul(gRotateCubeFace[index], float3(x, y, 0.5)));       // w
    return direction;
}


float RadicalInverseVDC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

/**
 * \brief    计算低差异序列随机数    
 * \param i  当前样本索引
 * \param N  样本数
 * \return   返回随机数
 */
float2 Hammersley(uint i, uint N) {
    return float2(float(i)/float(N), RadicalInverseVDC(i));
}

/**
 * \brief       		计算GGX重要性采样向量    
 * \param Xi    		低差异序列生成的随机数
 * \param N     		表面法线(归一化)
 * \param roughness		粗糙度
 * \param TBN           切线空间矩阵
 * \return      		采样向量
 */
static const float PI = 3.141592654;
float3 ImportanceSampleGGX(float2 Xi, float roughness, float3x3 TBN) {
	float a = roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTh = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTh = sqrt(1.0 - cosTh * cosTh);
    float cosPh = cos(phi);
    float sinPh = sin(phi);
    float3 H = {
		sinTh * cosPh,
        sinTh * sinPh,
        cosTh,
    };

    return mul(H, TBN);
}

static const uint kSampleCount = 4096;
SamplerState gSamLinearWrap          : register(s0);
TextureCube<float4>      gEnvMap     : register(t0);
RWTexture2DArray<float4> gOutputMip  : register(u0);

void SamplerGGXOnce(uint i, float3 N, float roughness, float3x3 TBN, inout float3 colorSum, inout float weightSum) {
    float3 V = N;
	float2 Xi = Hammersley(i, kSampleCount);
    float3 H = ImportanceSampleGGX(Xi, roughness, TBN);
    float3 L = normalize(2.0 * dot(V, H) * H - V);
    float NdotL = dot(N, L);
    if (NdotL > 0.0) {
	    colorSum += gEnvMap.SampleLevel(gSamLinearWrap, L, 0).rgb * NdotL;
        weightSum += NdotL;
    }
}

void CalcRoughness(ComputeIn cin, float3 N, float3x3 TBN) {
    float3 colorSum = 0.0;
    float weightSum = 0.0005;
    float roughness = fRoughness;
	for (uint i = 0; i < kSampleCount; ++i)
		SamplerGGXOnce(i, N, roughness, TBN ,colorSum, weightSum);
    gOutputMip[uint3(cin.DispatchThreadID.xy, gIndex)] = float4(colorSum / weightSum, 1.0);
}


[numthreads(16, 16, 1)]
void CS(ComputeIn cin) {
    float3 N = CalcDirection(cin);
    float3 up = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = normalize(cross(N, tangent));
    float3x3 TBN = float3x3(tangent, bitangent, N);
	CalcRoughness(cin, N, TBN);
}