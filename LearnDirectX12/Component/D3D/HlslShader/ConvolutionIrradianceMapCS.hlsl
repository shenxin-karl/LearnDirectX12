#line 2 "./LearnDirectX12/Component/D3D/HlslShader/ConvolutionIrradianceMapCS.hlsl"
cbuffer CBSettings : register(b0) {
    float gWidth;
    float gHeight;
    float gStep;
};

struct CSIn {
    uint3 	GroupID 		  : SV_GroupID;				// 线程组 ID
    uint3   GroupThreadID	  : SV_GroupThreadID;		// 组内线程 ID
    uint3   DispatchThreadID  : SV_DispatchThreadID;	// 线程 ID
    uint  	GroupIndex        : SV_GroupIndex;			// 线程组内索引
};

struct SHCoeffs {
	float3 m[9];
};

TextureCube<float4> gEnvMap           : register(t0);
RWStructuredBuffer<SHCoeffs> gOutput  : register(u0);
SamplerState gSamLinearClamp          : register(s0);

static float3x3 gRotateCubeFace[6] = {
    // +X
    float3x3(  0,  0,  1,
               0, -1,  0,
              -1,  0,  0 ),
    // -X
    float3x3(  0,  0, -1,
               0, -1,  0,
               1,  0,  0 ),
    // +Y
    float3x3(  1,  0,  0,
               0,  0,  1,
               0,  1,  0 ),
    // -Y
    float3x3(  1,  0,  0,
               0,  0, -1,
               0, -1,  0 ),
    // +Z
    float3x3(  1,  0,  0,
               0, -1,  0,
               0,  0,  1 ),
    // -Z
    float3x3( -1,  0,  0,
               0, -1,  0,
               0,  0, -1 )
};
float3 CalcDirection(CSIn csin) {
	uint index = csin.DispatchThreadID.z;
    float x = (csin.DispatchThreadID.x / gWidth)  - 0.5;
    float y = (csin.DispatchThreadID.y / gHeight) - 0.5;
    float3 direction = normalize(mul(gRotateCubeFace[index], float3(x, y, 0.5)));       // w
    return direction;
}

static const float PI = 3.141592654;
float3 CalcIrradiance(float3 direction) {
    float3 up = float3(0, 1, 0);
    float3 N = direction;
    float3 T = cross(N, up);
    float3 B = cross(T, N);
	       T = cross(N, B);

    float3 irradianceSum = float3(0, 0, 0);
    uint sampleCount = 0;
	const float delta = gStep;
	for (float phi = 0.0; phi < 2.0 * PI; phi += delta) {
	    for (float theta = 0.0; theta < 0.5 * PI; theta += delta) {
            float cosTh = cos(theta);
            float cosPh = cos(phi);
            float sinTh = 1.0 - cosTh * cosTh;
            float sinPh = 1.0 - cosPh * cosPh;
		    float3 V = float3(
				sinTh * cosPh,
                sinTh * sinPh,
                cosTh
            );
            float3 dir = (V.x * T) + (V.y * B) + (V.z * N);
            float3 irradiance = gEnvMap.SampleLevel(gSamLinearClamp, dir, 0).rgb;
            irradianceSum += irradiance * cosTh * sinTh;
            ++sampleCount;
	    }
    }
    return ((PI / sampleCount) * irradianceSum);
}


SHCoeffs CalcSphericalHarmonicsCoeff(float3 dir, float3 irradiance) {
	float x = dir.x; float y = dir.y; float z = dir.z;
    SHCoeffs sh3;
	sh3.m[0] = irradiance * 0.2820948;							// y0p0
	sh3.m[1] = irradiance * 0.4886025 * y;						// y1n1
	sh3.m[2] = irradiance * 0.4886025 * z;						// y1p0
	sh3.m[3] = irradiance * 0.4886025 * x;						// y1p1
	sh3.m[4] = irradiance * 1.0925480 * x * y;					// y2n2
	sh3.m[5] = irradiance * 1.0925480 * y * z;					// y2n1
	sh3.m[6] = irradiance * 0.3153916 * ((3.0 * z * z) - 1.0);	// y2p0
	sh3.m[7] = irradiance * 1.0925480 * x * z;					// y2p1
	sh3.m[8] = irradiance * 0.5462742 * (x * x - y * y);		// y2p2
    return sh3;
}

void SHCoeffsAccumulate(inout SHCoeffs lhs, SHCoeffs rhs) {
	[unroll(9)]
    for (int i = 0; i < 9; ++i)
		lhs.m[i] += rhs.m[i];
}

#define N 8
groupshared SHCoeffs gShCoeffs[N][N];
void StroeSHCoeffs(CSIn csin, SHCoeffs sh3) {
    int x = csin.GroupThreadID.x;
    int y = csin.GroupThreadID.y;
	// 每个线程执行
    gShCoeffs[x][y] = sh3;
	GroupMemoryBarrierWithGroupSync();
    
	//  x y 是2的倍数执行
    if ( ((x % 2) == 0 && (y % 2) == 0) ) {
		SHCoeffsAccumulate(sh3, gShCoeffs[x+1][y+0]);
		SHCoeffsAccumulate(sh3, gShCoeffs[x+0][y+1]);
		SHCoeffsAccumulate(sh3, gShCoeffs[x+1][y+1]);
	    gShCoeffs[x][y] = sh3;
    }        
    GroupMemoryBarrierWithGroupSync();

	//  x y 是4的倍数执行
    if ( ((x % 4) == 0 && (y % 4) == 0) ) {
	    SHCoeffsAccumulate(sh3, gShCoeffs[x+2][y+0]);
		SHCoeffsAccumulate(sh3, gShCoeffs[x+0][y+2]);
		SHCoeffsAccumulate(sh3, gShCoeffs[x+2][y+2]);
	    gShCoeffs[x][y] = sh3;
    }         
    GroupMemoryBarrierWithGroupSync();

    // 第一个线程执行
    if (csin.GroupIndex == 0) {
	    SHCoeffsAccumulate(sh3, gShCoeffs[x+4][y+0]);
		SHCoeffsAccumulate(sh3, gShCoeffs[x+0][y+4]);
		SHCoeffsAccumulate(sh3, gShCoeffs[x+4][y+4]);
        uint groupX = (uint)ceil((gWidth + 1) / N);
        uint groupY = (uint)ceil((gHeight + 1) / N);
        uint index = csin.GroupID.y * groupX + csin.GroupID.x;
        uint offset = csin.GroupID.z * (groupY * groupX);
        gOutput[offset + index] = sh3;
    }                  
}

[numthreads(N, N, 1)]
void CS(CSIn csin) {
	float3 direction = CalcDirection(csin);
    float3 irradiance = CalcIrradiance(direction);
    SHCoeffs sh3 = CalcSphericalHarmonicsCoeff(direction, irradiance);
    StroeSHCoeffs(csin, sh3);
}
