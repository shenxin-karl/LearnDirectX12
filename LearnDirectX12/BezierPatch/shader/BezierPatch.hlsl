#include "../../Component/D3D/HlslShader/ShaderCommon.hlsl"
#include "../../Component/D3D/HlslShader/LightingUtil.hlsl"

cbuffer CBObject :  register(b0) {
	float4x4     gWorld;
	MaterialData gMaterial;
};

cbuffer CBPass : register(b1) {
	CBPassType gPass;
};

cbuffer CBLight : register(b2) {
	CBLightType gLight;
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


PatchTess ConstantHS(InputPatch<VertexOut, 16> input) {
	PatchTess pt;
	pt.EdgeTess[0] = 25;
	pt.EdgeTess[1] = 25;
	pt.EdgeTess[2] = 25;
	pt.EdgeTess[3] = 25;
	pt.InsideTess[0] = 25;
	pt.InsideTess[1] = 25;
	return pt;
}

struct HullOut {
	float3 position : POSITION;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 16> input, uint i : SV_OutputControlPointID) {
	HullOut hout;
	hout.position = input[i].position;
	return hout;
}


float4 BernsteinBasis(float t) {
	float invT = 1.0 - t;
	return float4(
		invT * invT * invT,
		3.0 * t * invT * invT,
		3.0 * t * t * invT,
		t * t * t
	);
}

float4 dBernsteinBasis(float t) {
	float invT = 1.0 - t;
	return float4(
		-3.0 * invT * invT,
		3.0 * invT * invT - 6.0 * t * invT,
		6.0 * t * invT - 3.0 * t * t,
		3.0 * t * t
	);
}


float3 CubicBezierSum(const OutputPatch<HullOut, 16> bezpatch, float4 basisU, float4 basisV) {
	float3 sum = float3(0.0, 0.0, 0.0);
	sum += basisV.x * (bezpatch[0].position * basisU.x + bezpatch[1].position * basisU.y + 
					   bezpatch[2].position * basisU.z + bezpatch[3].position * basisU.w);
	
	sum += basisV.y * (bezpatch[4].position * basisU.x + bezpatch[5].position * basisU.y + 
					   bezpatch[6].position * basisU.z + bezpatch[7].position * basisU.w);
	
	sum += basisV.z * (bezpatch[8].position * basisU.x + bezpatch[9].position * basisU.y + 
					   bezpatch[10].position * basisU.z + bezpatch[11].position * basisU.w);
	
	sum += basisV.w * (bezpatch[12].position * basisU.x + bezpatch[13].position * basisU.y + 
					   bezpatch[14].position * basisU.z + bezpatch[15].position * basisU.w);
	return sum;
}

struct DomainOut {
	float4 SVPosition : SV_Position;
	float3 position   : POSITION;
	float3 normal     : NORMAL;
};

[domain("quad")]
DomainOut DS(PatchTess patchTess, 
			 float2 uv : SV_DomainLocation,		
             const OutputPatch<HullOut, 16> input)
{
	DomainOut dout;
	float4 basisU = BernsteinBasis(uv.x);
	float4 basisV = BernsteinBasis(uv.y);
	float3 position = CubicBezierSum(input, basisU, basisV);

	float4 dBasisU = dBernsteinBasis(uv.x);
	float4 dBasisV = dBernsteinBasis(uv.y);

	float3 tangent = CubicBezierSum(input, dBasisU, basisV);
	float3 bitangent = CubicBezierSum(input, basisU, dBasisV);

	float3 normal = normalize(cross(tangent, bitangent));
	float4 worldPosition = mul(gWorld, float4(position, 1.0));
	float3 worldNormal = mul(gWorld, float4(normal, 0.0)).xyz;

	dout.SVPosition = mul(gPass.viewProj, worldPosition);
	dout.position = worldPosition.xyz;
	dout.normal = worldNormal;
	return dout;
}

float4 PS(DomainOut pin) : SV_Target {
	float3 viewDir = gPass.eyePos - pin.position;
	float3 result = float3(0.0, 0.0, 0.0);
	float3 N = normalize(pin.normal);
	float3 V = normalize(viewDir);
	result += (gLight.ambientLight * gMaterial.diffuseAlbedo).xyz;
	result += ComputeDirectionLight(gLight.lights[0], gMaterial, N, V);
	result += ComputeDirectionLight(gLight.lights[1], gMaterial, N, V);
	result += ComputeDirectionLight(gLight.lights[2], gMaterial, N, V);
	return float4(result, gMaterial.diffuseAlbedo.a);
}