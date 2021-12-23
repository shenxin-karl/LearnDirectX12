#ifndef __COMMON_DEFINES__HLSL__
#define __COMMON_DEFINES__HLSL__

#define CB_OBJECT_TYPE      b0
#define CB_PASS_TYPE        b1
#define CB_MATERIAL_TYPE    b2

SamplerState samPointWrap        : register(s0);
SamplerState samPointClamp       : register(s1);
SamplerState samLinearWrap       : register(s2);
SamplerState samLinearClamp      : register(s3);
SamplerState samAnisotropicWrap  : register(s4);
SamplerState samAnisotropicClamp : register(s5);

#endif