<<<<<<< HEAD
=======

>>>>>>> f28e2c70cf10b53e093048452045bbb87e54af45
#include "LightHelper.hlsl"
 
cbuffer cbPerFrame : register(b0)
{
	DirectionLight gDirLights[3];
	float3 gEyePosW;
	int gLightCount;
	float4 gFogColor;
	float  gFogStart;
	float  gFogRange;
	bool gUseTexure;
};

cbuffer cbPerObject : register(b1)
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	Material gMaterial;
}; 

<<<<<<< HEAD
Texture2DArray gTreeMapArray : register(t0);
SamplerState gSamLinearClamp : register(s0);
=======
Texture2D gDiffuseMap : register(t0);
SamplerState gSamLinear : register(s0);
>>>>>>> f28e2c70cf10b53e093048452045bbb87e54af45

struct PS_INPUT
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
<<<<<<< HEAD
    float2 Tex     : TEXCOORD;
    uint   PrimID  : SV_PrimitiveID;
=======
	float2 Tex     : TEXCOORD;
>>>>>>> f28e2c70cf10b53e093048452045bbb87e54af45
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
<<<<<<< HEAD
		float3 uvw = float3(pin.Tex, 0); //pin.PrimID % 4);
		texColor = gTreeMapArray.Sample(gSamLinearClamp, uvw);
	
		clip(texColor.a - 0.05f);
=======
		texColor = gDiffuseMap.Sample(gSamLinear, pin.Tex);
>>>>>>> f28e2c70cf10b53e093048452045bbb87e54af45
	}
	 
	float4 litColor = texColor;
	if(gLightCount > 0)
	{  
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

<<<<<<< HEAD
		[unroll]
=======
>>>>>>> f28e2c70cf10b53e093048452045bbb87e54af45
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
}