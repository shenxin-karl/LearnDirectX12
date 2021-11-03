
cbuffer cUnifromBuffer : register(b0) {
	float4x4 gWorldViewProj;
};

struct VertexIn {
	float3 position : POSITION;
	float4 color	: COLOR;
};

struct VertexOut {
	float4 position : SV_Position;
	float4 color	: COLOR;
};

VertexOut VS(VertexIn vin) {
	VertexOut vout;
	vout.position = mul(float4(vin.position, 1.f), gWorldViewProj);
	vout.color    = vin.color;
    return vout;
}	

float4 PS(VertexOut pin) : SV_Target {
    return pin.color;
}