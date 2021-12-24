#ifndef __COMMON_DEFINES__HLSL__
#define __COMMON_DEFINES__HLSL__

// constant buffer type defines
#define CB_OBJECT_TYPE      b0
#define CB_PASS_TYPE        b1
#define CB_MATERIAL_TYPE    b2

// static sampler defines
SamplerState gSamPointWrap        : register(s0);
SamplerState gSamPointClamp       : register(s1);
SamplerState gSamLinearWrap       : register(s2);
SamplerState gSamLinearClamp      : register(s3);
SamplerState gSamAnisotropicWrap  : register(s4);
SamplerState gSamAnisotropicClamp : register(s5);

// texture resource type defines
#define TEXTURE_DIFFUSE_TYPE    t0
#define TEXTURE_ROUGHTNESS_TYPE t1
#define TEXTURE_METALLIC_TYPE   t2

#endif