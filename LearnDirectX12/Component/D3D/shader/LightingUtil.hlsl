#pragma once
#ifndef __LIGHTING_UTIL_HLSL_
#define __LIGHTING_UTIL_HLSL_

#define MAX_LIGHTS 16
struct Light {
    float3  strength;
    float   falloffStart;
    float3  direction;
    float   falloffEnd;
    float3  position;
    float   spotPower;
};

struct Material {
    float4 diffuseAlbedo;
    float3 fresnelR0;
    float  shiness;
};

float CalcAttenuation(float d, float falloffStart, float falloffEnd) {
    return saturate((falloffEnd - d) / ((falloffEnd - falloffStart)));
}

float CalcAttenuationSqr(float d, float falloffStart, float falloffEnd) {
    float dis = d - falloffStart;
    float lim = falloffEnd - falloffStart;
    dis *= dis;
    lim *= lim;
    return 1.0f - (dis / lim);
}

float3 SchilckFresnel(float3 R0, float3 N, float3 L) {
    float cosIndicentAngle = saturate(dot(N, L));
    float F0 = 1.0f - cosIndicentAngle;
    float refectPercent = R0 + (1.0f - R0) * (F0*F0*F0*F0*F0);
    return refectPercent;
}

float3 BlinnPhong(float3 lightStrength, float3 L, float3 N, float3 V, Material mat) {
    const float m = mat.shiness * 256.0f;
    float3 H = normalize(V + L);
    float normalizeFactor = (m + 8.0f) / 8.0f;
    float roughnessFactor = normalizeFactor * pow(saturate(dot(H, N)), m);
    float3 fresnelFactor = SchilckFresnel(mat.fresnelR0, N, L);
    float3 specAlbedo = fresnelFactor * roughnessFactor;
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);
    return (mat.diffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

float3 ComputeDirectLight(Light light, Material mat, float3 N, float3 V) {
    float3 L = -light.direction;
    float NdotL = saturate(dot(L, N));
    float3 lightStrength = light.strength * NdotL;
    return BlinnPhong(lightStrength, L, N, V, mat);
}

float3 ComputePointLight(Light light, Material mat, float3 wpos, float3 N, float3 V) {
    float3 lightVec = light.position - wpos;
    float d = length(lightVec);
    if (d > light.falloffEnd)
        return 0.0f;
        
    float3 L = lightVec / d;
    float NdotL = saturate(dot(N, L));
    float3 lightStrength = light.strength * NdotL;
    float att = CalcAttenuation(d, light.falloffStart, light.falloffEnd);
    lightStrength *= att;
    
    return BlinnPhong(lightStrength, L, N, V, mat);
}

float3 ComputeSpotLight(Light light, Material mat, float3 wpos, float3 N, float3 V) {
    float3 lightVec = light.position - wpos;
    float d = length(lightVec);
    if (d > light.falloffEnd)
        return 0.0;
        
    float3 L = lightVec / d;
    float NdotL = saturate(dot(N, L));
    float lightStrenght = light.strength * NdotL;

    float att = CalcAttenuation(d, light.falloffStart, light.falloffEnd);
    lightStrenght *= att;
    
    float spotFactor = pow(saturate(dot(-L, light.direction)), light.spotPower);
    lightStrenght *- spotFactor;
    
    return BlinnPhong(lightStrenght, L, N, V, mat);
}

float4 ComputeLighting(Light gLights[MAX_LIGHTS], Material mat, 
                       float3 wpos, float N, float V, 
                       float3 shadowFactor[MAX_LIGHTS]) {

    float3 result = 0.0f;
    int i = 0;
#if defined(NUM_DIR_LIGHTS) && (NUM_DIR_LIGHTS > 0)
    for ( ; i < NUM_DIR_LIGHTS; ++i)
        result += shadowFactor[i] * ComputeDirectLight(gLights[i], mat, N, V);
#endif
#if defined(NUM_POINT_LIGHTS) && (NUM_POINT_LIGHTS > 0)
    for ( ; i < NUM_DIR_LIGHTS+NUM_POINT_LIGHTS; ++i)
        result += shadowFactor[i] * ComputePointLight(gLights[i], mat, wpos, N, V);
#endif
#if defined(NUM_SPOT_LIGHTS) && (NUM_SPOT_LIGHTS > 0)
    for ( ; i < NUM_DIR_LIGHTS+NUM_POINT_LIGHTS+NUM_SPOT_LIGHTS; ++i)
        result += shadowFactor[i] * ComputeSpotLight(gLights[int], mat, wpos, N, V);
#endif
    return float4(result, 0.0f);
}

#endif