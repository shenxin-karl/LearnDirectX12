#include "../../Component/D3D/shader/ShaderCommon.hlsl"

cbuffer CBObject : register(b0) {
	float4x4 gWorld;
};

cbuffer CBPass : register(b1) {
    PassCBType gPass;
};

cbuffer CBLight : register(b2) {
	LightCBType gLight;
};


struct VertexIn {
	float3 position : POSITION;
};

struct VertexOut {
	float3 position : POSITION;
};

VertexOut VS(VertexIn vin) {
	VertexOut vout;
	vout.position = vin.position;
	return vout;
}

struct PatchTess {
	float EdgeTess[4]   : SV_TessFactor;
	float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 4> patch, uint patchID : SV_PrimitiveID) {
	PatchTess pt;
	float3 centerL = (patch[0].position + patch[1].position + patch[2].position + patch[3].position) * 0.25;
	float3 centerW = mul(gWorld, float4(centerL, 1.0));
	float dis = distance(centerW, gPass.eyePos);

	const float dis0 = 20.0;
	const float dis1 = 100.0;
	float tess = max(64.0 * saturate((dis1-dis) / (dis1-dis0)), 1.0);
	pt.EdgeTess[0] = tess;
	pt.EdgeTess[1] = tess;
	pt.EdgeTess[2] = tess;
	pt.EdgeTess[3] = tess;
	pt.InsideTess[0] = tess;
	pt.InsideTess[1] = tess;
	return pt;
}


struct HullOut {
	float3 position : POSITION;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 4> input, 
		   uint i : SV_OutputControlPointID,
		   uint patchID : SV_PrimitiveID)
{
	HullOut hout;
	hout.position = input[i].position;
	return hout;
}