
struct Direction {
	float phi;
	float theta;
};

struct SH3Coeff {
	float3 m[9];
};

struct SH3BasisFunction {
	float c[9];
};

TextureCube<float4>					gEnvCubeMap : register(t0);
ConsumeStructuredBuffer<Direction>	gInput		: register(t1);
AppendStructuredBuffer<SH3Coeff>	gOutput		: register(t2);
SamplerState gSamLinearClamp					: register(s0);

float3 DirectionToVector3(Direction dir) {
	float cosTh = cos(dir.theta); float cosPh = cos(dir.phi);
	float sinTh = sin(dir.theta); float sinPh = sin(dir.phi);
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
	float3 dir = DirectionToVector3(gInput.Consume());
	float3 L = gEnvCubeMap.SampleLevel(gSamLinearClamp, dir, 0);
	SH3BasisFunction basis = CalcSHBasisFunction(dir);
	SH3Coeff coeff;

	[unroll(9)]
	for (int i = 0; i < 9; ++i)
		coeff.m[i] = basis.c[i] * L;

	gOutput.Append(coeff);
}