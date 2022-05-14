#ifndef _COLOR_GRADING_HLSL_
#define _COLOR_GRADING_HLSL_

float3 ACESToneMapping(float3 color, float adaptedLum) {
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;
	color *= adaptedLum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}

float3 GammaCorrection(float3 color, float gamma = 2.2) {
	float invGamma = 1.f / gamma;
	return pow(color, float3(invGamma, invGamma, invGamma));
}

#endif