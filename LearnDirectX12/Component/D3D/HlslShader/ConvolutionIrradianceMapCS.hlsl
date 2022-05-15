#line 2 "./LearnDirectX12/Component/D3D/HlslShader/ConvolutionIrradianceMapCS.hlsl"
cbuffer CBSettings : register(b0) {
    float gWidth;
    float gHeight;
    float gStep;
};

struct ComputeIn {
    uint3 	GroupID 		  : SV_GroupID;				// �߳��� ID
    uint3   GroupThreadID	  : SV_GroupThreadID;		// �����߳� ID
    uint3   DispatchThreadID  : SV_DispatchThreadID;	// �߳� ID
    uint  	GroupIndex        : SV_GroupIndex;			// �߳���������
};

struct SHCoeffs {
	float3 m[9];
};

TextureCube<float4> gEnvMap           : register(t0);
RWStructuredBuffer<SHCoeffs> gOutput  : register(u0);
SamplerState gSamLinearClamp          : register(s0);

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

static const float PI = 3.141592654;
float3 CalcIrradiance(float3 direction) {
    float3 N = direction;
    float3 up 		 = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangent 	 = cross(up, N);
    float3 bitangent = cross(N, tangent);
    return bitangent * 0.5 + 0.5;

    float3x3 TBN = float3x3(tangent, bitangent, N);
    float3 irradianceSum = float3(0, 0, 0);
    uint sampleCount = 0;
	const float delta = gStep;
	for (float phi = 0.0; phi < 2.0 * PI; phi += delta) {
	    for (float theta = 0.0; theta < 0.5 * PI; theta += delta) {
            float cosTh = cos(theta);
            float cosPh = cos(phi);
            float sinTh = sqrt(1.0 - cosTh * cosTh);
            float sinPh = sqrt(1.0 - cosPh * cosPh);
		    float3 V = float3(
				sinTh * cosPh,
                sinTh * sinPh,
                cosTh
            );
            float3 dir = mul(V, TBN);
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
void StroeSHCoeffs(ComputeIn cin, SHCoeffs sh3) {
    int x = cin.GroupThreadID.x;
    int y = cin.GroupThreadID.y;
	// ÿ���߳�ִ��
    gShCoeffs[x][y] = sh3;
	GroupMemoryBarrierWithGroupSync();
    
	//  x y ��2�ı���ִ��
    if ( ((x % 2) == 0 && (y % 2) == 0) ) {
		SHCoeffsAccumulate(sh3, gShCoeffs[x+1][y+0]);
		SHCoeffsAccumulate(sh3, gShCoeffs[x+0][y+1]);
		SHCoeffsAccumulate(sh3, gShCoeffs[x+1][y+1]);
	    gShCoeffs[x][y] = sh3;
    }        
    GroupMemoryBarrierWithGroupSync();

	//  x y ��4�ı���ִ��
    if ( ((x % 4) == 0 && (y % 4) == 0) ) {
	    SHCoeffsAccumulate(sh3, gShCoeffs[x+2][y+0]);
		SHCoeffsAccumulate(sh3, gShCoeffs[x+0][y+2]);
		SHCoeffsAccumulate(sh3, gShCoeffs[x+2][y+2]);
	    gShCoeffs[x][y] = sh3;
    }         
    GroupMemoryBarrierWithGroupSync();

    // ��һ���߳�ִ��
    if (cin.GroupIndex == 0) {
	    SHCoeffsAccumulate(sh3, gShCoeffs[x+4][y+0]);
		SHCoeffsAccumulate(sh3, gShCoeffs[x+0][y+4]);
		SHCoeffsAccumulate(sh3, gShCoeffs[x+4][y+4]);
        uint groupX = (uint)ceil((gWidth + 1) / N);
        uint groupY = (uint)ceil((gHeight + 1) / N);
        uint index = cin.GroupID.y * groupX + cin.GroupID.x;
        uint offset = cin.GroupID.z * (groupY * groupX);
        gOutput[offset + index] = sh3;
    }                  
}

[numthreads(N, N, 1)]
void CS(ComputeIn cin) {
	float3 direction = CalcDirection(cin);
    float3 irradiance = CalcIrradiance(direction);
    SHCoeffs sh3 = CalcSphericalHarmonicsCoeff(direction, irradiance);
    StroeSHCoeffs(cin, sh3);
}
