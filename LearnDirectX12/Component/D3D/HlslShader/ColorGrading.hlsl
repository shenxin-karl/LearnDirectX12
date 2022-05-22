#ifndef _COLOR_GRADING_HLSL_
#define _COLOR_GRADING_HLSL_

float3 ACESToneMapping(float3 color, float adaptedLum) 
{
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;
	color *= adaptedLum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}

float3 Uncharted2ToneMappingImpl(float3 x) {
	const float A = 0.22f;
	const float B = 0.30f;
	const float C = 0.10f;
	const float D = 0.20f;
	const float E = 0.01f;
	const float F = 0.30f;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 Uncharted2ToneMapping(float3 color, float adapted_lum) {
	const float WHITE = 11.2f;
	return Uncharted2ToneMappingImpl(1.6f * adapted_lum * color) / Uncharted2ToneMappingImpl(WHITE);
}

float3 CEToneMapping(float3 color, float adapted_lum) {
    return 1 - exp(-adapted_lum * color);
}

float3 GammaCorrection(float3 color, float gamma = 2.2) {
	float invGamma = 1.f / gamma;
	return pow(color, float3(invGamma, invGamma, invGamma));
}

float3 ConvertToLinear(float3 x) {
    return x < 0.04045f ? x / 12.92 : pow((x + 0.055) / 1.055, 2.4);
}

float3 ConvertToSRGB(float3 x) {
    return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;
}

#endif
