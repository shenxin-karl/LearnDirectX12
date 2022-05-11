
struct RandomNumberPair {
	float u;
	float v;
};

struct SH3Coeff {
	float3 m[9];
};

struct SH3BasisFunction {
	float c[9];
};

TextureCube<float4>							gEnvCubeMap : register(t0);
ConsumeStructuredBuffer<RandomNumberPair>	gInput		: register(u0);
AppendStructuredBuffer<SH3Coeff>			gOutput		: register(u1);
SamplerState gSamLinearClamp							: register(s0);

const static float PI = 3.141592654;
float3 RandomNumberPairToVector3(RandomNumberPair pair) {
	float theta = 2.0 * acos(sqrt(1.0 - pair.u));
	float phi = 2.0 * PI * pair.v;
	float cosTh = cos(theta); float cosPh = cos(phi);
	float sinTh = sin(theta); float sinPh = sin(phi);
	return float3(
		sinTh * cosPh,
		cosTh,
		sinTh * sinPh
	);
}

SH3BasisFunction CalcSHBasisFunction(float3 v) {
	float x = v.x;
	float y = v.y;
	float z = v.z;
	SH3BasisFunction basis;
	basis.c[0] = 0.2820948;							// y0p0
	basis.c[1] = 0.4886025 * y;						// y1n1
	basis.c[2] = 0.4886025 * z;						// y1p0
	basis.c[3] = 0.4886025 * x;						// y1p1
	basis.c[4] = 1.0925480 * x * y;					// y2n2
	basis.c[5] = 1.0925480 * y * z;					// y2n1
	basis.c[6] = 0.3153916 * (3.0 * z * z) - 1.0;	// y2p0
	basis.c[7] = 1.0925480 * x * z;					// y2p1
	basis.c[8] = 0.5462742 * (x * x - y * y);		// y2p2
	return basis;
}

[numthreads(16, 1, 1)]
void CS() {
	float3 dir = RandomNumberPairToVector3(gInput.Consume());
	float3 L = gEnvCubeMap.SampleLevel(gSamLinearClamp, dir, 0);
	SH3BasisFunction basis = CalcSHBasisFunction(dir);
	SH3Coeff coeff;

	[unroll(9)]
	for (int i = 0; i < 9; ++i)
		coeff.m[i] = basis.c[i] * L;

	gOutput.Append(coeff);
}