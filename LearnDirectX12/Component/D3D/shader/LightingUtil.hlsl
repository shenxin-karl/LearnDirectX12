#ifndef __LIGHTING_UTIL_HLSL__
#define __LIGHTING_UTIL_HLSL__

/* macro defines:
 * USE_CARTOON_SHADING:    使用卡通着色
 * NUM_DIR_LIGHTS:         直接光照光源数量
 * NUM_POINT_LIGHTS:       点光源数量
 * NUM_SPOT_LIGHTS:        聚光灯数量
*/

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
    float  metallic;
};

#ifndef USE_CARTOON_SHADING
    #define DIFF_SHADING_FACTOR(v) (v)
    #define SPEC_SHADING_FACTOR(v) (v)
#else

float CarToonDiffShadingFactor(float NdotL) {
    if (NdotL <= 0.f)
        return 0.4f;
    else if (NdotL <= 0.5f)
        return 0.6f;
    else
        return 1.0f;
}    
    
float CarToonSpecShadingFactor(float HdotN) {
    if (HdotN <= 0.1f)
        return 0.0f;
    else if (HdotN <= 0.8f)
        return 0.5f;
    else
        return 0.8f;
}
    #define DIFF_SHADING_FACTOR(v) CarToonDiffShadingFactor(v)
    #define SPEC_SHADING_FACTOR(v) CarToonSpecShadingFactor(v)
#endif

float CalcAttenuation(float d, float falloffStart, float falloffEnd) {
    return saturate((falloffEnd - d) / ((falloffEnd - falloffStart)));
}

float CalcAttenuationSqr(float dis) {
    return 1.0f / (dis * dis);
}

float3 SchlickFresnel(float3 R0, float3 H, float3 L) {
    float cosIndicentAngle = saturate(dot(H, L));
    float F0 = 1.0f - cosIndicentAngle;
    float3 refectPercent = R0 + (1.0f - R0) * (F0*F0*F0*F0*F0);
    return refectPercent;
}

float3 BlinnPhong(float3 lightStrength, float3 L, float3 N, float3 V, Material mat) {
    float  metallic = saturate(mat.metallic);
    float3 diffuse  = mat.diffuseAlbedo.rgb * (1.0f - metallic);
    float3 R0       = lerp(float3(0.04f, 0.04f, 0.04f), mat.fresnelR0, metallic);
    
    const float m = max(mat.shiness * 256.0f, 1.0f);
    float3 H = normalize(V + L);
    float normalizeFactor = (m + 8.0f) / 8.0f;
    float roughnessFactor = normalizeFactor * SPEC_SHADING_FACTOR(pow(saturate(dot(H, N)), m));
    float3 fresnelFactor = SchlickFresnel(R0, H, L);
    float3 specAlbedo = fresnelFactor * roughnessFactor;
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);
    return (mat.diffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

float3 ComputeDirectLight(Light light, Material mat, float3 N, float3 V) {
    float3 L = -light.direction;
    float NdotL = saturate(DIFF_SHADING_FACTOR(dot(L, N)));
    float3 lightStrength = light.strength * NdotL;
    return BlinnPhong(lightStrength, L, N, V, mat);
}

float3 ComputePointLight(Light light, Material mat, float3 wpos, float3 N, float3 V) {
    float3 lightVec = light.position - wpos;
    float d = length(lightVec);
    if (d > light.falloffEnd)
        return 0.0f;
        
    float3 L = lightVec / d;
    float NdotL = saturate(DIFF_SHADING_FACTOR(dot(N, L)));
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
    float NdotL = saturate(DIFF_SHADING_FACTOR(dot(N, L)));
    float3 lightStrenght = light.strength * NdotL;

    float att = CalcAttenuation(d, light.falloffStart, light.falloffEnd);
    lightStrenght *= att;
    
    float spotFactor = pow(saturate(dot(-L, light.direction)), light.spotPower);
    lightStrenght *- spotFactor;
    
    return BlinnPhong(lightStrenght, L, N, V, mat);
}

#ifndef NUM_DIR_LIGHTS 
    #define NUM_DIR_LIGHTS 1
#endif

float4 ComputeLighting(Light gLights[MAX_LIGHTS], Material mat, 
                       float3 wpos, float3 N, float3 V, 
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