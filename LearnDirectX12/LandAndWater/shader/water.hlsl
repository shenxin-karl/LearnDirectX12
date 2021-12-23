#include "../../Component/D3D/shader/CommonDefines.hlsl"
#include "../../Component/D3D/shader/LightingUtil.hlsl"
#include "../../Component/D3D/shader/PassConstantBuffer.hlsl"
#include "../../Component/D3D/shader/MaterialConstantBuffer.hlsl"

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

struct WaveParam {
    float length;
    float omega;
    float phi;
    float amplitude;
    float WA;
    float Q;
    float3 dir;
};

void MakeWaveImpl(in float3 wpos, in WaveParam wp, out float3 pos, out float3 nrm) {
    float theta = wp.omega * dot(wp.dir.xz, wpos.xz) + wp.phi * gTotalTime;
    float sinTh = sin(theta);
    float cosTh = cos(theta);
    float QA    = wp.Q * wp.amplitude;
    float3 position = {
        QA * wp.dir.x * cosTh,
        wp.amplitude * sinTh,
        QA * wp.dir.z * cosTh,
    };
    float3 normal = {
        wp.dir.x * wp.WA * cosTh,
        wp.Q * wp.WA * sinTh,
        wp.dir.z * wp.WA * cosTh,
    };
    
    pos = position;
    nrm = normal;
}

static const float k_PI = 3.141592654;
static const float k_2PI = k_PI * 2.f;
static const float WaveLength = 50.f;
static const float WaveAmplitude = 1.2f;
static const int WaveNumber = 4;
void MakeWave(inout float3 wpos, inout float3 wnrm) {
    const float l0 = 0.5 * WaveLength; const float w0 = k_2PI / l0; const float p0 = 0.8f * w0;
    const float a0 = 0.5f * WaveAmplitude; const float wa0 = w0 * a0; const float q0 = 0.8f / (wa0 * WaveNumber); 
    float3 dir0 = { +0.31f, 0.0f, +0.69f }; float3 pos0; float3 nrm0;
    WaveParam wp0 = { l0, w0, p0, a0, wa0, q0, dir0 };
    MakeWaveImpl(wpos, wp0, pos0, nrm0);
    
    const float l1 = 0.6 * WaveLength; const float w1 = k_2PI / l1; const float p1 = 3.f * w1;
    const float a1 = 0.3f * WaveAmplitude; const float wa1 = w1 * a1; const float q1 = 1.0f / (wa1 * WaveNumber);
    float3 dir1 = { -0.4f, 0.0f, -0.6f }; float3 pos1; float3 nrm1;
    WaveParam wp1 = { l1, w1, p1, a1, wa1, q1, dir1 };
    MakeWaveImpl(wpos, wp1, pos1, nrm1);
    
    const float l2 = 0.7 * WaveLength; const float w2 = k_2PI / l2; const float p2 = 1.9f * w2;
    const float a2 = 0.5f * WaveAmplitude; const float wa2 = w2 * a2; const float q2 = 0.3f / (wa2 * WaveNumber);
    float3 dir2 = { +0.58f, 0.0f, -0.42f }; float3 pos2; float3 nrm2;
    WaveParam wp2 = { l2, w2, p2, a2, wa2, q2, dir2 };
    MakeWaveImpl(wpos, wp2, pos2, nrm2);
    
    const float l3 = 0.3 * WaveLength; const float w3 = k_2PI / l3; const float p3 = 2.5f * w3;
    const float a3 = 0.7f * WaveAmplitude; const float wa3 = w3 * a3; const float q3 = 0.5f / (wa3 * WaveNumber);
    float3 dir3 = { -0.43f, 0.0f, +0.57f }; float3 pos3; float3 nrm3;
    WaveParam wp3 = { l3, w3, p3, a3, wa3, q3, dir3 };
    MakeWaveImpl(wpos, wp3, pos3, nrm3);
    
    float3 pos = pos0 + pos1 + pos2 + pos3;
    float3 nrm = nrm0 + nrm1 + nrm2 + nrm3;
    
    wpos = float3(wpos.x, 0, wpos.z) + pos;
    wnrm = float3(0.f, 1.f, 0.f) - nrm;
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

float4 PS(VertexOut pin) : SV_Target {
    Material mat = {
        gDiffuseAlbedo,
        gFresnelR0,
        1.0f - gRoughness,
        gMetallic,
    };
    
    float3 N = normalize(pin.wnrm);
    float3 V = normalize(gEyePos - pin.wpos);
    
    float3 shadowFactor[16];
    shadowFactor[0] = float3(1.f, 1.f, 1.f);
    float3 result = ComputeLighting(gLights, mat, pin.wpos, N, V, shadowFactor);
    return float4(result, gDiffuseAlbedo.a);
}