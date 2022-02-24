#include "../../Component/D3D/shader/ShaderCommon.hlsl"
#include "../../Component/D3D/shader/LightingUtil.hlsl"

struct WaterParame {
    float  length;      // 波长
    float  omega;       // 角频率
    float  speed;       // 波数; 相常数表示法 
    float  amplitude;   // 振幅
    float3 direction;   // 方向
    float  steep;       // 陡峭度
};

cbuffer CPassCB : register(b0) {
    PassCBType   gPass;
    WaterParame  gWaterParames[4];
};

cbuffer CLightCB : register(b1) {  
    LightCBType  gLight;
};

cbuffer CObjectCB : register(b2) {
    float4x4 gWorld;
    Material gMaterial;
};

struct VertexIn {
    float3 position : POSITION;
    float3 normal   : NORMAL;
};

struct VertexOut {
    float4 SVPosition : SV_Position;
    float3 wpos       : POSITION;
    float3 wnrm       : NORMAL;
};

void MakeWaveImpl(float3 wpos, WaterParame waveParame, out float3 pos, out float3 nrm) {
    float WA    = waveParame.omega * waveParame.amplitude;
    float QA    = waveParame.steep * waveParame.amplitude;
    float theta = waveParame.omega * dot(waveParame.direction.xz, wpos.xz) + waveParame.speed * gPass.totalTime;
    float sinTh = sin(theta);
    float cosTh = cos(theta);
    float3 position = {
        waveParame.direction.x * cosTh * QA,
        waveParame.amplitude   * sinTh,
        waveParame.direction.z * cosTh * QA,
    };
    float3 normal = {
        waveParame.direction.x * WA * cosTh,
        waveParame.steep       * WA * sinTh,
        waveParame.direction.z * WA * cosTh,
    };
    pos = position;
    nrm = normal;
}

void MakeWave(inout float3 wpos, inout float3 wnrm) {
    float3 worldPosition = wpos;
    wpos = float3(0.0, 0.0, 0.0);
    wnrm = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < 4; ++i) {
        float3 wavePosRet;
        float3 waveNrmRet;
        MakeWaveImpl(worldPosition, gWaterParames[i], wavePosRet, waveNrmRet);
        wpos += wavePosRet;
        wnrm += waveNrmRet;
    }
    
    wpos = float3(wpos.x, 0.0, wpos.z) + worldPosition;
    wnrm = float3(0.0, 1.0, 0.0) = wnrm;
}

VertexOut VS(VertexIn vin) {
    float4 worldPosition = mul(gWorld, float4(vin.position, 1.0));
    float3 wpos = worldPosition.xyz;
    float3 wnrm = vin.normal;
    MakeWave(wpos, wnrm);
    VertexOut vout;
    vout.SVPosition = mul(gPass.viewProj, worldPosition);
    vout.wpos = wpos;
    vout.wnrm = wnrm;
    return vout;
}

float4 PS(VertexOut pin) : SV_Target {
    float4 result  = { 0.0, 0.0, 0.0, 0.0 };
    float3 viewDir = gPass.eyePos - pin.wpos;
    result += ComputeDirectionLight(gLight.lights[0], gMaterial, pin.wnrm, viewDir);
    result += gMaterial.diffuseAlbedo * gLight.ambientLight;
    return result;
}