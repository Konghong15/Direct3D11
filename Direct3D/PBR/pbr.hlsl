static const float PI = 3.141592;
static const float Epsilon = 0.00001;
static const uint NumLights = 3;
static const float3 Fdielectric = 0.04;

cbuffer cbVertexShader : register(b0)
{
	float4x4 viewProjectionMatrix;
	float4x4 skyProjectionMatrix;
	float4x4 sceneRotationMatrix;
};

cbuffer cbPixelShader : register(b0)
{
	struct {
		float4 direction;
		float4 radiance;
	} lights[NumLights];
	float4 eyePosition;	
    float4 useIBL;
};

struct VertexShaderInput
{
	float3 position  : POSITION;
	float3 normal    : NORMAL;
	float3 tangent   : TANGENT;
	float3 bitangent : BITANGENT;
	float2 texcoord  : TEXCOORD;
};

struct PixelShaderInput
{
	float4 pixelPosition : SV_POSITION;
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
	float3x3 tangentBasis : TBASIS;
};

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metalnessTexture : register(t2);
Texture2D roughnessTexture : register(t3);
TextureCube specularTexture : register(t4);
TextureCube irradianceTexture : register(t5);
Texture2D specularBRDF_LUT : register(t6); // 룩업 테이블

SamplerState defaultSampler : register(s0);
SamplerState spBRDF_Sampler : register(s1);

float ndfGGX(float ndoth, float roughness)
{
	float alpha   = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (ndoth * ndoth) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

float gaSchlickGGX(float ndotl, float ndotv, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(ndotl, k) * gaSchlickG1(ndotv, k);
}

float3 fresnelSchlick(float3 F0, float cosTheta)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

uint querySpecularTextureLevels()
{
	uint width, height, levels;
	specularTexture.GetDimensions(0, width, height, levels);
	return levels; 
}

PixelShaderInput VS(VertexShaderInput vin)
{
	PixelShaderInput vout;
	
	vout.position = mul(float4(vin.position, 1.0), sceneRotationMatrix).xyz;
	vout.texcoord = float2(vin.texcoord.x, 1.0 - vin.texcoord.y);

	float3x3 TBN = float3x3(vin.tangent, vin.bitangent, vin.normal);
	vout.tangentBasis = mul(transpose(TBN), (float3x3)sceneRotationMatrix);

	float4x4 mvpMatrix = mul(sceneRotationMatrix, viewProjectionMatrix);
	vout.pixelPosition = mul(float4(vin.position, 1.0), mvpMatrix);

	return vout;
}

float4 PS(PixelShaderInput pin) : SV_Target
{
	float3 albedo = albedoTexture.Sample(defaultSampler, pin.texcoord).rgb;
	float metalness = metalnessTexture.Sample(defaultSampler, pin.texcoord).r;
	float roughness = roughnessTexture.Sample(defaultSampler, pin.texcoord).r;

	float3 view = normalize(eyePosition - pin.position);

	float3 N = normalize(2.0 * normalTexture.Sample(defaultSampler, pin.texcoord).rgb - 1.0);
	N = normalize(mul(pin.tangentBasis, N));
	
	float ndotv = max(0.0, dot(N, view));
	
	float3 viewReflect = 2.0 * ndotv * N - view;

	// 프레넬 반사율
	float3 F0 = lerp(Fdielectric, albedo, metalness);

	float3 directLighting = 0.0;
	for(uint i=0; i<NumLights; ++i)
	{
		float3 lightInv = -lights[i].direction;
		float3 lradiance = lights[i].radiance;

		float3 half = normalize(lightInv + view);

		float ndotl = max(0.0, dot(N, lightInv));
		float ndoth = max(0.0, dot(N, half));

		float3 F  = fresnelSchlick(F0, max(0.0, dot(half, view))); // 프레넬
		float D = ndfGGX(ndoth, max(Epsilon, roughness)); // 정반사 분포도
		float G = gaSchlickGGX(ndotl, ndotv, roughness); // 기하 감쇠율
		float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * ndotl * ndotv);

		float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metalness);
		float3 diffuseBRDF = kd * albedo;

		directLighting += (diffuseBRDF + specularBRDF) * lradiance * ndotl;
	}

	float3 ambientLighting;
    if (useIBL.r > 0)		
	{
		float3 irradiance = irradianceTexture.Sample(defaultSampler, N).rgb; // 난반사 조도, 노말로 샘플링
		uint specularTextureLevels = querySpecularTextureLevels();
		float3 specularIrradiance = specularTexture.SampleLevel(defaultSampler, viewReflect, roughness * specularTextureLevels).rgb; // 정반사 조도, 뷰 반사벡터로 샘플링, 거칠기를 반영함?
		float2 specularBRDF = specularBRDF_LUT.Sample(spBRDF_Sampler, float2(ndotv, roughness)).rg; // 정반사 BRDF 룩업테이블, 노말 뷰의 cos값과 거칠기로 샘플링

		float3 F = fresnelSchlick(F0, ndotv); // 주변 조명에 대한 프레넬
		float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

		float3 kd = lerp(1.0 - F, 0.0, metalness); // 확산 기여 계수??
		float3 diffuseIBL = kd * albedo * irradiance;

		ambientLighting = diffuseIBL + specularIBL;
	}

	return float4(directLighting + ambientLighting, 1.0);
}
