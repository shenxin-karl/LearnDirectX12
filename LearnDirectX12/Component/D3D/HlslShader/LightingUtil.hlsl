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
    return NdotL <= 0.f ? 0.4 : (NdotL <= 0.5 ? 0.6 : 1.0);
}    
    
float CarToonSpecShadingFactor(float HdotN) {
    return NdotH <= 0.1 ? 0.0 : (HdotN <= 0.8 ? 0.5 : 0.8);
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

#ifndef _DECLARE_SH3_
#define _DECLARE_SH3_
struct SH3 {
	float4 y0p0;
	float4 y1n1; float4 y1p0; float4 y1p1;
	float4 y2n2; float4 y2n1; float4 y2p0; float4 y2p1; float4 y2p2;
};
#endif

float CalcAttenuation(float d, float falloffStart, float falloffEnd) {
    return 1.0 - saturate((d - falloffStart) / (falloffEnd - falloffStart));
}

float CalcFogAttenuation(float d, float fogStart, float fogEnd) {
    return saturate((d - fogStart) / (fogEnd - fogStart));
}

float3 SchlickFresnel(float3 F0, float cosIncidenceAngle) {
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
    float3 freshnelFactor = SchlickFresnel(F0, saturate(dot(H, L)));
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

/**
 * \brief       获取辐照度    
 * \param sh    IrradianceMap 的球谐拟合系数            
 * \param N     表面法线(归一化)
 * \return      辐照度
 */
float3 SampleSH(SH3 sh, float3 N) {
	float4 result = 0.0;
    float x = N.x;      float y = N.y;      float z = N.z;

    // l = 0
    result += sh.y0p0 * 0.2820948;
    // l = 1
    result += sh.y1n1 * 0.4886025 * y;
    result += sh.y1p0 * 0.4886025 * z;
    result += sh.y1p1 * 0.4886025 * x;
    // l = 2
    result += sh.y2n2 * 1.0925480 * x * y;
    result += sh.y2n1 * 1.0925480 * y * z;
    result += sh.y2p0 * 0.3153916 * ((3.0 * z * z) - 1.0);
    result += sh.y2p1 * 1.0925480 * x * z;
    result += sh.y2p2 * 0.5462742 * (x*x - y*y);
    return (float3)result;
}

float3 SchlickFresnelRoughness(float cosIncidenceAngle, float3 R0, float roughness) {
    float cosTh = 1.0 - cosIncidenceAngle;
    float3 reflected = R0 + (max(1.0-roughness, R0) - R0) * (cosTh * cosTh * cosTh * cosTh * cosTh);
    return reflected;
}

struct IBLParam {
    SH3          sh3;
	SamplerState texSampler;
    Texture2D    brdfLutMap;
    TextureCube  perFilterEnvMap;
};

float3 CalcIBLAmbient(float3 V, float3 N, Material material, IBLParam param) {
    float3 albedo = (float3)material.diffuseAlbedo;
	float3 R0 = lerp(0.04, albedo, material.metallic);
    float NdotV = max(dot(N, V), 0);
    float3 F = SchlickFresnelRoughness(NdotV, R0, material.roughness);
    float3 Ks = F;

    // diffuse
    float3 Kd = max((1.0 - Ks), 0) * (1.0 - material.metallic);
    float3 irradiance = SampleSH(param.sh3, N);
    float3 diffuse = Kd * irradiance * albedo;

    // specular
    const float MAX_REFLECTION_LOD = 4.0;
    float3 R = reflect(-V, N);
    float lod = material.roughness * MAX_REFLECTION_LOD;
    float2 texcoord = float2(NdotV, material.roughness);
    float2 envBRDF = param.brdfLutMap.SampleLevel(param.texSampler, texcoord, 0).xy;
    float3 perFilterColor = param.perFilterEnvMap.SampleLevel(param.texSampler, R, lod).rgb;
    float3 specular = perFilterColor * (F * envBRDF.x + envBRDF.y);

    return diffuse + specular;
}

#endif