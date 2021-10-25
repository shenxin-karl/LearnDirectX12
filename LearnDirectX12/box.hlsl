
cbuffer cUnifromBuffer : register(b0) {
	float4x4 gWorldViewProj;
};

struct VertexIn {
	float3 position : POSITIONT;
	float3 color	: COLOR;
};

struct VertexOut {
	float4 position : SV_Position;
	float3 color	: COLOR;
};

void VS(VertexIn vin) {
	VertexOut vout;
	vout.position = mul(float4(vin.position, 1.f), gWorldViewProj);
	vout.color    = vin.color;
}	

float4 PS(VertexOut pin) : SV_Target {
	return float4(pin.color, 1.f);
}