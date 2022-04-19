#ifndef __LIGHTING_UTIL_HLSL__
#define __LIGHTING_UTIL_HLSL__

/*
开启卡通着色
#define USE_CARTOON_SHADING  
*/

#ifndef USE_CARTOON_SHADING
    #define DIFF_SHADING_FACTOR(NdotL) (NdotL)
    #define SPEC_SHADING_FACTOR(NdotH) (NdotH)
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
    #define DIFF_SHADING_FACTOR(NdotL) CarToonDiffShadingFactor(NdotL)
    #define SPEC_SHADING_FACTOR(NdotH) CarToonSpecShadingFactor(NdotH)
#endif


#ifndef _DECLARE_LIGHT_
#define _DECLARE_LIGHT_
struct Light {
    float3 strength;        // 辐射强度
    float  falloffStart;    // 点光源/聚光灯:衰减开始距离
    float3 direction;       // 方向光/聚光灯:光源方向
    float  falloffEnd;      // 点光源/聚光灯:衰减结束距离
    float3 position;        // 点光源位置
    float  spotPower;       // 聚光灯 pow 指数
};
#endif

#ifndef _DECLARE_MATERIAL_
#define _DECLARE_MATERIAL_
struct Material {
    float4 diffuseAlbedo;   // 反照率
    float  roughness;       // 粗糙度
    float  metallic;        // 金属度
    float  padding0;         // 填充0
    float  pading1;         // 填充1
};
#endif

float CalcAttenuation(float d, float falloffStart, float falloffEnd) {
    return 1.0 - saturate((d - falloffStart) / (falloffEnd - falloffStart));
}

float CalcFogAttenuation(float d, float fogStart, float fogEnd) {
    return saturate((d - fogStart) / (fogEnd - fogStart));
}

float3 SclickFresnel(float3 F0, float cosIncidenceAngle) {
    float cosTh = 1.0 - cosIncidenceAngle;
    return F0 + (1.f - F0) * (cosTh * cosTh * cosTh * cosTh * cosTh);
}

float3 BlinnPhong(float3 lightStrength, float3 L, float3 N, float3 V, Material mat) {
    float m = max((1.f - mat.roughness) * 256.f, 1.f);
    float3 H = normalize(V + L);
    
    // Diffuse material R0 is 0.04
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), mat.diffuseAlbedo.rgb, mat.metallic);
    
    float NdotH = saturate(dot(N, H));
    float roughnessFactor = (m + 2.f) / 8.0 * pow(NdotH, m);
    float3 freshnelFactor = SclickFresnel(F0, saturate(dot(H, L)));
    float3 specAlbedo = SPEC_SHADING_FACTOR(roughnessFactor) * freshnelFactor;
    
    // If the material is metal, the diffuse reflection will be reduced
    float3 diffAlbedo = mat.diffuseAlbedo.rgb * (1.f - mat.metallic);
    
    // Make sure it's between 0 and 1
    specAlbedo = specAlbedo / (specAlbedo + 1.f);
    return (diffAlbedo + specAlbedo) * lightStrength;
}

float3 ComputeDirectionLight(Light light, Material mat, float3 normal, float3 viewDir) {
    float3 V = normalize(viewDir);
    float3 N = normalize(normal);
    float3 L = light.direction;
    float NdotL = DIFF_SHADING_FACTOR(saturate(dot(N, L)));
    float3 lightStrength = light.strength * NdotL;
    return BlinnPhong(lightStrength, L, N, V, mat);
}

float3 ComputePointLight(Light light, Material mat, float3 normal, float3 viewDir, float3 worldPosition) {
    float3 lightVec = worldPosition - light.position;
    float dis = length(lightVec);
    if (dis > light.falloffEnd)
        return 0.f;
    
    float3 V = normalize(viewDir);
    float3 N = normalize(normal);
    float3 L = lightVec / dis;
    float NdotL = DIFF_SHADING_FACTOR(saturate(dot(N, L)));
    float attenuation = CalcAttenuation(dis, light.falloffStart, light.falloffEnd);
    float3 lightStrength = light.strength * NdotL * attenuation;
    return BlinnPhong(lightStrength, L, N, V, mat);
}

float3 ComputeSpotLight(Light light, Material mat, float3 normal, float3 viewDir, float3 worldPosition) {
    float3 lightVec = light.position - worldPosition;
    float dis = length(lightVec);
    if (dis > light.falloffEnd)
        return 0.f;
        
    float3 V = normalize(viewDir);
    float3 N = normalize(normal);
    float3 L = lightVec / dis;
    float NdotL = DIFF_SHADING_FACTOR(saturate(dot(N, L)));
    float3 lightStrength = light.strength * NdotL;
    
    float attenuation = CalcAttenuation(dis, light.falloffStart, light.falloffEnd);
    lightStrength *= attenuation;
    
    float spotFactor = pow(saturate(dot(-L, light.direction)), light.spotPower);
    lightStrength *= spotFactor;

    return BlinnPhong(lightStrength, L, N, V, mat);
}

#endif