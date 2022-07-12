#include "../../Component/D3D/HlslShader/ShaderCommon.hlsl"

struct VertexIn {
	float3 position : POSITION;
};

struct VertexOut {
	float4 SVPosition : SV_Position;
};

cbuffer CbObject : register(b0) {
	float4x4	 gMatWorld;
	float4x4	 gMatNormal;
	float4x4	 gMatTexCoord;
	MaterialData gMaterialData;
};

cbuffer CbPass : register(b1) {
	CBPassType gPass;
}

VertexOut VS(VertexIn vin) {
	VertexOut vout;
	float4 worldPosition = mul(gMatWorld, float4(vin.position, 1.0));
	vout.SVPosition = mul(gPass.lightSpaceMatrix, worldPosition);
	return vout;
}

void PS(VertexOut pin) {
	
}