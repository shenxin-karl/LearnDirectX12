#ifndef __MATERIAL_CONSTANT_BUFFER_HLSL__
#define __MATERIAL_CONSTANT_BUFFER_HLSL__

cbuffer MaterialConstants : register(CB_MATERIAL_TYPE) {
    float4      gDiffuseAlbedo;
    float3      gFresnelR0; 
    float       gMatPad1;            // ���
    float       gRoughness;
    float       gMetallic;           
    float2      gMatPad2;            // ���2
    float4x4    gMatTransfrom;
};


#endif