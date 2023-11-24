<<<<<<< HEAD
=======

>>>>>>> f28e2c70cf10b53e093048452045bbb87e54af45
#include "LightHelper.hlsl"

cbuffer cbPerObject : register(b0)
{
<<<<<<< HEAD
	float4x4 gViewProj;
	Material gMaterial;
};

struct VS_INPUT
{
	float3 PosW  : POSITION;
	float2 SizeW : SIZE;
=======
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	Material gMaterial;
}; 

struct VS_INPUT
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
>>>>>>> f28e2c70cf10b53e093048452045bbb87e54af45
};

struct VS_OUTPUT
{
<<<<<<< HEAD
	float3 CenterW : POSITION;
	float2 SizeW   : SIZE;
=======
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	float2 Tex     : TEXCOORD;
>>>>>>> f28e2c70cf10b53e093048452045bbb87e54af45
};

VS_OUTPUT main(VS_INPUT vin)
{
	VS_OUTPUT vout;
<<<<<<< HEAD

	vout.CenterW = vin.PosW;
	vout.SizeW   = vin.SizeW;
=======
	
	vout.PosW    = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
		
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
>>>>>>> f28e2c70cf10b53e093048452045bbb87e54af45

	return vout;
}