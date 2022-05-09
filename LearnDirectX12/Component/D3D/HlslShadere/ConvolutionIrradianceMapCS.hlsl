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

TextureCube<float4> gEnvMap      : register(t0);
RWTexture2DArray<float4> gOutput : register(u0);
SamplerState gSamLinearClamp     : register(s0);

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

static const float PI = 3.141592654;

[numthreads(32, 32, 1)]
void CS(CSIn csin) {
    uint index = csin.DispatchThreadID.z;
    float x = (csin.DispatchThreadID.x / gWidth)  - 0.5;
    float y = (csin.DispatchThreadID.y / gHeight) - 0.5;
    float3 direction = normalize(mul(gRotateCubeFace[index], float3(x, y, 0.5)));       // w

    float3 v = direction.y > 0.95 ? float3(1, 0, 0) : float3(0, 1, 0);
    float3 u = cross(v, direction);
    float3 w = cross(u, v);

    float3 irradianceSum = float3(0, 0, 0);
    uint sampleCount = 0;
	const float delta = gStep;
	for (float phi = 0.0; phi < 2.0 * PI; phi += delta) {
	    for (float theta = 0.0; theta < 0.5 * PI; theta += delta) {
            float cosTh = cos(theta);
            float sinTh = sin(theta);
            float cosPh = cos(phi);
            float sinPh = sin(phi);
		    float3 V = float3(
				sinPh * cosTh,
                cosPh,
                sinPh * sinTh
            );
            float3 dir = (V.x * u) + (V.y * v) + (V.z * w);
            float3 irradiance = gEnvMap.SampleLevel(gSamLinearClamp, dir, 0);
            irradianceSum += irradiance * cosTh * sinTh;
            ++sampleCount;
	    }
    }

    gOutput[csin.DispatchThreadID.xyz] = float4(PI * irradianceSum * (1.0 / sampleCount), 1.0);
}
