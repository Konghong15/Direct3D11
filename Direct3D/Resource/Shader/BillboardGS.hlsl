<<<<<<< HEAD
#include "LightHelper.hlsl"

cbuffer cbPerObject : register(b0)
{
	float4x4 gViewProj;
	Material gMaterial;
};

cbuffer cbPerFrame : register(b1)
=======

#include "LightHelper.hlsl"
 
cbuffer cbPerFrame : register(b0)
>>>>>>> f28e2c70cf10b53e093048452045bbb87e54af45
{
	DirectionLight gDirLights[3];
	float3 gEyePosW;
	int gLightCount;
	float4 gFogColor;
	float  gFogStart;
	float  gFogRange;
	bool gUseTexure;
};

<<<<<<< HEAD
cbuffer cbFixed
{
	float2 gTexC[4] = 
	{
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};
};

struct GeoIn
{
	float3 CenterW : POSITION;
	float2 SizeW   : SIZE;
};

struct GeoOut
=======
cbuffer cbPerObject : register(b1)
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	Material gMaterial;
}; 

Texture2D gDiffuseMap : register(t0);
SamplerState gSamLinear : register(s0);

struct PS_INPUT
>>>>>>> f28e2c70cf10b53e093048452045bbb87e54af45
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
<<<<<<< HEAD
    float2 Tex     : TEXCOORD;
    uint   PrimID  : SV_PrimitiveID;
};

[maxvertexcount(4)]
void main(point GeoIn gin[1]
	, uint primID : SV_PrimitiveID
	, inout TriangleStream<GeoOut> triStream)
{
	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 look = gEyePosW - gin[0].CenterW;
	look.y = 0.0f;
	look = normalize(look);
	float3 right = cross(up, look);

	float halfWidth  = 0.5f * gin[0].SizeW.x;
	float halfHeight = 0.5f * gin[0].SizeW.y;

	// 각 축에 대한 선형 결합으로 평면 좌표를 형성함 
	float4 v[4];
	v[0] = float4(gin[0].CenterW + halfWidth * right - halfHeight * up, 1.0f);
	v[1] = float4(gin[0].CenterW + halfWidth * right + halfHeight * up, 1.0f);
	v[2] = float4(gin[0].CenterW - halfWidth * right - halfHeight * up, 1.0f);
	v[3] = float4(gin[0].CenterW - halfWidth * right + halfHeight * up, 1.0f);


	float2 gTexC[4] = 
	{
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};

	GeoOut gout;
	[unroll]
	for(int i = 0; i < 4; ++i)
	{
		gout.PosH     = mul(v[i], gViewProj);
		gout.PosW     = v[i].xyz;
		gout.NormalW  = look;
		gout.Tex      = gTexC[i];
		gout.PrimID   = primID;
		
		triStream.Append(gout);
	}

	// triStream.RestartStrip();
=======
	float2 Tex     : TEXCOORD;
};

float4 main(PS_INPUT pin) : SV_Target
{
    pin.NormalW = normalize(pin.NormalW);

	float3 toEye = gEyePosW - pin.PosW;

	float distToEye = length(toEye); 
	
	toEye /= distToEye;
	
    float4 texColor = float4(1, 1, 1, 1);
    if(gUseTexure)
	{
		texColor = gDiffuseMap.Sample(gSamLinear, pin.Tex);
	}
	 
	float4 litColor = texColor;
	if(gLightCount > 0)
	{  
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

		for(int i = 0; i < gLightCount; ++i)
		{
			float4 A, D, S;
			ComputeDirectionLight(gMaterial, gDirLights[i], pin.NormalW, toEye, A, D, S);

			ambient += A;
			diffuse += D;
			spec    += S;
		}

		litColor =  texColor * (ambient + diffuse) + spec;
	}

	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
>>>>>>> f28e2c70cf10b53e093048452045bbb87e54af45
}