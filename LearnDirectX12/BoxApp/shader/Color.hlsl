
struct VertexIn {
    float3 position : POSITION;
    float4 color    : COLOR;
};

struct VertexOut {
    float4 position : SV_Position;
    float4 color    : COLOR;
};

cbuffer ConstantBuffer : register(b0){
    float4x4 gWorldViewProj;
};

VertexOut VS(VertexIn vin) {
    VertexOut vout;
    //vout.position = mul(gWorldViewProj, float4(vin.position, 1.0));
    vout.position = float4(vin.position.xy, 0.0, 1.0);
    vout.color = vin.color;
    return vout;
}

float4 PS(VertexOut pin) : SV_Target {
    return pin.color;
}