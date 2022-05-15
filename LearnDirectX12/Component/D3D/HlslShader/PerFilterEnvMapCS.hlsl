cbuffer CBSettgings : register(b0) {
	float gWidth;
    float gHeight;
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
	uint index = cin.DispatchThreadID.z;
    float x = (cin.DispatchThreadID.x / gWidth)  - 0.5;
    float y = (cin.DispatchThreadID.y / gHeight) - 0.5;
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
float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness, float3x3 TBN) {
    float a = roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTh = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float cosPh = cos(phi);
    float sinTh = sqrt(1.0 - cosTh * cosTh);
    float sinPh = sqrt(1.0 - cosPh * cosPh);
    float3 H = {			// 生成切线空间中的半程向量
        sinPh * cosTh,
        sinPh * sinTh,
        cosPh,
    };
    
    // 转到采样空间
    float3 sampleVec = mul(TBN, H);
    return normalize(sampleVec);
}

static const uint kSampleCount = 1024;
SamplerState gSamLinearWrap          : register(s0);
TextureCube<float4>      gEnvMap     : register(t0);
RWTexture2DArray<float4> gOutputMip0 : register(u0);
RWTexture2DArray<float4> gOutputMip1 : register(u1);
RWTexture2DArray<float4> gOutputMip2 : register(u2);
RWTexture2DArray<float4> gOutputMip3 : register(u3);
RWTexture2DArray<float4> gOutputMip4 : register(u4);


void SamplerGGXOnce(uint i, float3 N, float roughness, float3x3 TBN, inout float3 colorSum, inout float weightSum) {
    float3 V = N;
	float2 Xi = Hammersley(i, kSampleCount);
    float3 H = ImportanceSampleGGX(Xi, N, roughness, TBN);
    float3 L = normalize(2.0 * dot(V, H) * H - V);
    float NdotL = dot(N, L);
    if (NdotL > 0.0) {
	    colorSum += gEnvMap.Sample(gSamLinearWrap, L).rgb * NdotL;
        weightSum += NdotL;
    }
}

static const float k3Zero = float3(0, 0, 0);

void CalcRoughness1(ComputeIn cin, float3 N, float3x3 TBN) {
    float3 colorSum = 0.0;
    float weightSum = 0.0;
    float roughness = 0.0;
	for (uint i = 0; i < kSampleCount; ++i)
		SamplerGGXOnce(i, N, roughness, TBN ,colorSum, weightSum);

    gOutputMip0[cin.DispatchThreadID.xyz] = float4(colorSum / weightSum, 1.0);
}

void CalcRoughness2(ComputeIn cin, float3 N, float3x3 TBN) {
	float3 colorSumList[2] = { k3Zero, k3Zero };
    float weightSumList[2] = { 0.0, 0.0 };
    float roughnessList[2] = { 0.0, 0.25 * 0.25 };
    for (uint i = 0; i < kSampleCount; ++i) {
		SamplerGGXOnce(i, N, roughnessList[0], TBN ,colorSumList[0], weightSumList[0]);
		SamplerGGXOnce(i, N, roughnessList[1], TBN ,colorSumList[1], weightSumList[1]);
    }
    gOutputMip0[cin.DispatchThreadID.xyz] = float4(colorSumList[0] / weightSumList[0], 1.0);
    uint3 index1 = uint3(cin.DispatchThreadID.xy / 2, cin.DispatchThreadID.z);
    gOutputMip1[index1] = float4(colorSumList[1] / weightSumList[1], 1.0);
}

void CalcRoughness3(ComputeIn cin, float3 N, float3x3 TBN) {
	float3 colorSumList[3] = { k3Zero, k3Zero, k3Zero };
    float weightSumList[3] = { 0.0, 0.0, 0.0 };
    float roughnessList[3] = { 0.0, 0.25 * 0.25, 0.5*0.5 };
    for (uint i = 0; i < kSampleCount; ++i) {
		SamplerGGXOnce(i, N, roughnessList[0], TBN ,colorSumList[0], weightSumList[0]);
		SamplerGGXOnce(i, N, roughnessList[1], TBN ,colorSumList[1], weightSumList[1]);
		SamplerGGXOnce(i, N, roughnessList[2], TBN ,colorSumList[2], weightSumList[2]);
    }
    gOutputMip0[cin.DispatchThreadID.xyz] = float4(colorSumList[0] / weightSumList[0], 1.0);
    uint3 index1 = uint3(cin.DispatchThreadID.xy / 2, cin.DispatchThreadID.z);
    gOutputMip1[index1] = float4(colorSumList[1] / weightSumList[1], 1.0);
    uint3 index2 = uint3(cin.DispatchThreadID.xy / 4, cin.DispatchThreadID.z);
    gOutputMip2[index2] = float4(colorSumList[2] / weightSumList[2], 1.0);
}

void CalcRoughness4(ComputeIn cin, float3 N, float3x3 TBN) {
	float3 colorSumList[4] = { k3Zero, k3Zero, k3Zero, k3Zero };
    float weightSumList[4] = { 0.0, 0.0, 0.0, 0.0 };
    float roughnessList[4] = { 0.0, 0.25 * 0.25, 0.5*0.5, 0.75*0.75 };
    for (uint i = 0; i < kSampleCount; ++i) {
		SamplerGGXOnce(i, N, roughnessList[0], TBN ,colorSumList[0], weightSumList[0]);
		SamplerGGXOnce(i, N, roughnessList[1], TBN ,colorSumList[1], weightSumList[1]);
		SamplerGGXOnce(i, N, roughnessList[2], TBN ,colorSumList[2], weightSumList[2]);
		SamplerGGXOnce(i, N, roughnessList[3], TBN ,colorSumList[3], weightSumList[3]);
    }
    gOutputMip0[cin.DispatchThreadID.xyz] = float4(colorSumList[0] / weightSumList[0], 1.0);
    uint3 index1 = uint3(cin.DispatchThreadID.xy / 2, cin.DispatchThreadID.z);
    gOutputMip1[index1] = float4(colorSumList[1] / weightSumList[1], 1.0);
    uint3 index2 = uint3(cin.DispatchThreadID.xy / 4, cin.DispatchThreadID.z);
    gOutputMip2[index2] = float4(colorSumList[2] / weightSumList[2], 1.0);
	uint3 index3 = uint3(cin.DispatchThreadID.xy / 8, cin.DispatchThreadID.z);
    gOutputMip3[index3] = float4(colorSumList[3] / weightSumList[3], 1.0);
}

void CalcRoughness5(ComputeIn cin, float3 N, float3x3 TBN) {
	float3 colorSumList[5] = { k3Zero, k3Zero, k3Zero, k3Zero, k3Zero };
    float weightSumList[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
    float roughnessList[5] = { 0.0, 0.25 * 0.25, 0.5*0.5, 0.75*0.75, 1.0 };
    for (uint i = 0; i < kSampleCount; ++i) {
		SamplerGGXOnce(i, N, roughnessList[0], TBN ,colorSumList[0], weightSumList[0]);
		SamplerGGXOnce(i, N, roughnessList[1], TBN ,colorSumList[1], weightSumList[1]);
		SamplerGGXOnce(i, N, roughnessList[2], TBN ,colorSumList[2], weightSumList[2]);
		SamplerGGXOnce(i, N, roughnessList[3], TBN ,colorSumList[3], weightSumList[3]);
		SamplerGGXOnce(i, N, roughnessList[4], TBN ,colorSumList[4], weightSumList[4]);
    }
    gOutputMip0[cin.DispatchThreadID.xyz] = float4(colorSumList[0] / weightSumList[0], 1.0);
    uint3 index1 = uint3(cin.DispatchThreadID.xy / 2, cin.DispatchThreadID.z);
    gOutputMip1[index1] = float4(colorSumList[1] / weightSumList[1], 1.0);
    uint3 index2 = uint3(cin.DispatchThreadID.xy / 4, cin.DispatchThreadID.z);
    gOutputMip2[index2] = float4(colorSumList[2] / weightSumList[2], 1.0);
	uint3 index3 = uint3(cin.DispatchThreadID.xy / 8, cin.DispatchThreadID.z);
    gOutputMip3[index3] = float4(colorSumList[3] / weightSumList[3], 1.0);
	uint3 index4 = uint3(cin.DispatchThreadID.xy / 16, cin.DispatchThreadID.z);
    gOutputMip4[index4] = float4(colorSumList[4] / weightSumList[4], 1.0);
}


[numthreads(16, 16, 1)]
void CS(ComputeIn cin) {
	float3 N         = CalcDirection(cin);
    float3 up 		 = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangent 	 = cross(up, N);
    float3 bitangent = cross(N, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, N);
    
    if (cin.GroupIndex == 0)
		CalcRoughness5(cin, N, TBN);
    else if (cin.GroupThreadID.x % 8 == 0 && cin.GroupThreadID.y % 8 == 0)
		CalcRoughness4(cin, N, TBN);
    else if (cin.GroupThreadID.x % 4 == 0 && cin.GroupThreadID.y % 4 == 0)
		CalcRoughness3(cin, N, TBN);
    else if (cin.GroupThreadID.x % 2 == 0 && cin.GroupThreadID.y % 2 == 0)
		CalcRoughness2(cin, N, TBN);
    else
		CalcRoughness1(cin, N, TBN);
}