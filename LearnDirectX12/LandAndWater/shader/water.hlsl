#include "../../Component/D3D/shader/PassConstantBuffer.hlsl"

struct VertexIn {
    float3 position : POSITION;
	float3 normal   : NORMAL;
};

struct VertexOut {
	float4 SVPosition : SV_Position;
	float3 wpos		  : POSITION;
	float3 wnrm		  : NORMAL;
};

cbuffer ObjCBBuffer : register(b0) {
    float4x4 gWorld;
};

static const float k_PI = 3.141592654;
static const float k_2PI = k_PI * 2;
static const float WaveLength = 10.f;
static const float WaveAmplitude = 0.1f;
static const float WaveNumber = 1;
static const float3 WaveDir = normalize(float3(0.5, 0, 0.5));
void MakeWave(inout float3 wpos, inout float3 wnrm) {
    float speed = 2.0f;
    float phi = 1.0 * k_2PI / WaveLength;
    float w = k_2PI / WaveLength;
    float WA = w * WaveAmplitude;
    float Q = 0.8f;
    float qi = Q / (WA * WaveNumber);
    float QA = qi * WaveAmplitude;
    float theta = (w * dot(WaveDir.xz, wpos.xz) + phi * gTotalTime);
    float cosTh = cos(theta);
    float sinTh = sin(theta);

    float3 wavePos = {
        wpos.x + (QA * WaveDir.x * cosTh),
        WaveAmplitude * sinTh,
        wpos.z + (QA * WaveDir.z * cosTh),
    };
    float3 waveNrm = {
        -1 * WaveDir.x * WA * cosTh,
        +1 - qi * WA * sinTh,
        -1 * WaveDir.z * WA * cosTh,
    };

    wpos = wavePos;
    wnrm = waveNrm;
}

VertexOut VS(VertexIn vin) {
    float4 worldPosition = mul(gWorld, float4(vin.position, 1.0));
	float3 wpos = worldPosition.xyz;
	float3 wnrm = vin.normal;
	MakeWave(wpos, wnrm);
	VertexOut vout;
    vout.SVPosition = mul(gViewProj, float4(wpos, 1.0));
	vout.wpos	    = wpos;
	vout.wnrm		= wnrm;
	return vout;
}

static const float3 albedo = { 135.f / 255.f, 206.f / 255.f, 250.f / 255.f };
float4 PS(VertexOut pin) : SV_Target {
	return float4(albedo, 1.0);
    //float3 N = normalize(pin.wnrm);
    //return float4(N*0.5+0.5, 1.0);
}