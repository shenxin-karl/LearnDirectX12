#line 2 "./LearnDirectX12/Component/D3D/HlslShader/PanoToCubeMapCS.hlsl"
cbuffer CBSettings : register(b0) {
    float gWidth;
    float gHeight;
};

struct ComputeIn {
    uint3 	GroupID 		  : SV_GroupID;				// �߳��� ID
    uint3   GroupThreadID	  : SV_GroupThreadID;		// �����߳� ID
    uint3   DispatchThreadID  : SV_DispatchThreadID;	// �߳� ID
    uint  	GroupIndex        : SV_GroupIndex;			// �߳���������
};

Texture2D gPannoEnvMap              : register(t0);
RWTexture2DArray<float4> gOutput    : register(u0);
SamplerState gSamLinearClamp        : register(s0);

// (0, 0, 1)  ��ת�� 6 ������, �ֱ��� +x, -x, +y, -y, +z, -z
// ����ʹ�õ��� mul( Matrix, Vector ) ���Ծ������о���
// ʹ�õ�����������ϵ
const static float3x3 gRotateCubeFace[6] = {
    float3x3(float3(+0,  +0, +1), float3(+0, -1, +0), float3(-1, +0, +0) ),   // +X
    float3x3(float3(+0,  +0, -1), float3(+0, -1, +0), float3(+1, +0, +0) ),   // -X
    float3x3(float3(+1,  +0, +0), float3(+0, +0, +1), float3(+0, +1, +0) ),   // +Y
    float3x3(float3(+1,  +0, +0), float3(+0, +0, -1), float3(+0, -1, +0) ),   // -Y
    float3x3(float3(+1,  +0, +0), float3(+0, -1, +0), float3(+0, +0, +1) ),   // +Z
    float3x3(float3(-1,  +0, +0), float3(+0, -1, +0), float3(+0, +0, -1) )    // -Z
};

static const float PI = 3.141592654;
static const float2 invTan = float2(0.5 / PI, 1.0 / PI);
float2 SamplerPanoramaMap(float3 v) {
	float2 uv = float2(atan2(v.z, v.x), -asin(v.y));
	uv *= invTan;
	uv += 0.5;
	return uv;
}

[numthreads(32, 32, 1)]
void CS(ComputeIn csin) {
    uint index = csin.DispatchThreadID.z;
    float x = (csin.DispatchThreadID.x / gWidth)  - 0.5;
    float y = (csin.DispatchThreadID.y / gHeight) - 0.5;
    float3 target = float3(x, y, 0.5);
    float3 direction = normalize(mul(gRotateCubeFace[index], target));
    float2 texcoord = SamplerPanoramaMap(direction);
	gOutput[csin.DispatchThreadID.xyz] = gPannoEnvMap.SampleLevel(gSamLinearClamp, texcoord, 0);
}