#include "../../Component/D3D/shader/PassConstantBuffer.hlsl"

struct VertexIn {
    float3 position : POSITION;
	float3 normal   : NORMAL;
};

struct VertexOut {
	float4 SVPosition : SV_Position;
	float3 wpos		  : POSITION;
	float3 wnrm		  : NORMAL;
};

cbuffer ObjCBBuffer : register(b0) {
    float4x4 gWorld;
};

void MakeWave(inout float3 wpos, inout float3 wnrm) {
	
}

VertexOut VS(VertexIn vin) {
	float4 worldPosition = mul(float4(vin.position, 1.0) ,gWorld);
	float3 wpos = worldPosition.xyz;
	float3 wnrm = vin.normal;
	MakeWave(wpos, wnrm);
	VertexOut vout;
	vout.SVPosition = mul(float4(wpos, 1.0), gViewProj);
	vout.wpos	    = wpos;
	vout.wnrm		= wnrm;
	return vout;
}

//float3 albedo = float3(135.f / 255.f, 206.f / 255.f, 250.f / 255.f);
float4 PS(VertexOut pin) : SV_Target {
	//return float4(albedo, 1.0);
    return float4(0, 0, 0, 0);

}