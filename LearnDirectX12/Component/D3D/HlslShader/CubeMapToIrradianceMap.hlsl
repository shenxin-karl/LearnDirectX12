
cbuffer cbPreCamera : register(b0) {
	float4x4 gViewProj;
    float    gRoughness;
};

struct VertexIn {
	float3 position : POSITION;
};

struct VertexOut {
	float4 SVPosition    : SV_POSITION;
	float3 localPosition : POSITION;
};

VertexOut VS(VertexIn vin) {
	VertexOut vout;
	vout.SVPosition = mul(gViewProj, vin.position);
	vout.localPosition = vin.position;
	return vout;
}

TextureCube gEnvMap         : register(t0);
SamplerState gSamLinearWrap	: register(s0);

static const float PI = 3.141592654;
static const float Step = 0.025;
float4 PS(VertexOut pin) : SV_Target {
	float3 N         = normalize(pin.localPosition);
    float3 up 		 = abs(N.y) < 1.0 ? float3(0, 1, 0) : float3(0, 0, 1);
    float3 tangent 	 = cross(up, N);
    float3 bitangent = cross(N, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, N);

    float3 irradianceSum = float3(0, 0, 0);
    uint sampleCount = 0;
	for (float phi = 0.0; phi < 2.0 * PI; phi += Step) {
        float cosPh = cos(phi);
        float sinPh = sin(phi);
	    for (float theta = 0.0; theta < 0.5 * PI; theta += Step) {
            float cosTh = cos(theta);
			float sinTh = sin(theta);
		    float3 V = float3(
				sinTh * cosPh,
                sinTh * sinPh,
                cosTh
            );
            float3 dir = mul(V, TBN);
            float3 irradiance = gEnvMap.SampleLevel(gSamLinearWrap, dir, 0).rgb;
            irradianceSum += irradiance * cosTh * sinTh;
            ++sampleCount;
	    }
    }
    return float4(((PI / sampleCount) * irradianceSum), 1.0);
}