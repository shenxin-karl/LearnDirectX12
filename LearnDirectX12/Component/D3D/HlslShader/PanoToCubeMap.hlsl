
cbuffer CBPass {
	float4x4 gViewProj;
};

struct VertexIn {
	float3 position : POSITION;
};

struct VertexOut {
	float4 SVPosition : SV_Position;
	float3 position   : POSITION;
};

VertexOut VS(VertexIn vin) {
	VertexOut vout;
	vout.SVPosition = mul(gViewProj, float4(vin.position, 1.0));
	vout.position   = vin.position;
	return vout;
}

static const float PI = 3.141592654;
static const float2 invTan = float2(0.5 / PI, 1.0 / PI);
float2 SamplerPanoramaMap(float3 v) {
	float2 uv = float2(atan2(v.z, v.x), -asin(v.y));
	uv *= invTan;
	uv += 0.5;
	return uv;
}

Texture2D<float4> gPanoramaMap    : register(t0); 
SamplerState      gSamLinearClamp : register(s0);
float4 PS(VertexOut pin) : SV_Target {
	float3 direction = normalize(pin.position);
	float2 texcoord = SamplerPanoramaMap(direction);
	return gPanoramaMap.Sample(gSamLinearClamp, texcoord);
}