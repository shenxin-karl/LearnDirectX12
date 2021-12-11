#ifndef __MATERIAL_CONSTANT_BUFFER_HLSL__
#define __MATERIAL_CONSTANT_BUFFER_HLSL__

cbuffer MaterialConstants : register(CB_MATERIAL_TYPE) {
    float4      diffuseAlbedo;
    float3      fresnelR0; 
    float       matPad1;            // 野割
    float       roughness;
    float       metallic;           
    float2      matPad2;            // 野割2
    float4x4    matTransfrom;
};


#endif