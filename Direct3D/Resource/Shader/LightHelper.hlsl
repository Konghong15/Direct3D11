struct LightElement
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
};

struct DirectionLight
{
	LightElement Intensity;
	float3 Direction;
	float pad;
};

struct PointLight
{
	LightElement Intensity;
	float3 Position;
	float Range;
	float3 AttenuationParam;
	float pad;
};

struct SpotLight
{
	LightElement Intensity;
	float3 Direction;
	float Spot;
	float3 Position;
	float Range;
	float3 AttenuationParam;
	float pad;
};

struct Material
{
	LightElement ReflectionIntesity; // specular w = specular power
	float4 Reflect;
};

void ComputeDirectionLight(Material material
	, DirectionLight directionLight
	, float3 normal
	, float3 toEye
	, out float4 ambient
	, out float4 diffuse
	, out float4 specular)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// 간접광
	ambient = material.ReflectionIntesity.Ambient * directionLight.Intensity.Ambient;

	// 난반사광
	float ndotl = dot(normal, -directionLight.Direction);

	if (ndotl < 0)
	{
		return;
	}

	diffuse = material.ReflectionIntesity.Diffuse * directionLight.Intensity.Diffuse * ndotl;

	// 정반사광
	float3 r = reflect(directionLight.Direction, normal);
	float rdotv = dot(r, normalize(toEye));
	specular = material.ReflectionIntesity.Specular * directionLight.Intensity.Specular * pow(max(rdotv, 0.0f), directionLight.Intensity.Specular.w);
}

void ComputePointLight(Material material
	, PointLight pointLight
	, float3 position
	, float3 normal
	, float3 toEye
	, out float4 ambient
	, out float4 diffuse
	, out float4 specular)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightDirection = position - pointLight.Position;
	float distance = length(lightDirection);

	if (distance > pointLight.Range)
	{
		return;
	}

	// 주변광
	ambient = material.ReflectionIntesity.Ambient * pointLight.Intensity.Ambient;

	// 난반사광
	lightDirection /= distance;
	float ndotl = dot(normal, -lightDirection);

	if (ndotl < 0)
	{
		return;
	}

	diffuse = material.ReflectionIntesity.Diffuse * pointLight.Intensity.Diffuse * ndotl;

	// 정반사광
	float3 r = reflect(lightDirection, normal);
	float rdotv = dot(r, normalize(toEye));
	specular = material.ReflectionIntesity.Specular * pointLight.Intensity.Specular * pow(max(rdotv, 0.0f), pointLight.Intensity.Specular.w);

	// 감쇠율
	float invAttenuationRate = 1 / dot(pointLight.AttenuationParam, float3(1.0f, distance, distance * distance));
	diffuse *= invAttenuationRate;
	specular *= invAttenuationRate;
}

void ComputeSpotLight(Material material
	, SpotLight spotLight
	, float3 position
	, float3 normal
	, float3 toEye
	, out float4 ambient
	, out float4 diffuse
	, out float4 specular)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightDirection = position - spotLight.Position;
	float distance = length(lightDirection);

	if (distance > spotLight.Range)
	{
		return;
	}

	// 주변광
	ambient = material.ReflectionIntesity.Ambient * spotLight.Intensity.Ambient;

	// 난반사광
	lightDirection /= distance;
	float ndotl = dot(normal, -lightDirection);

	if (ndotl < 0)
	{
		return;
	}

	diffuse = material.ReflectionIntesity.Diffuse * spotLight.Intensity.Diffuse * ndotl;

	// 정반사광
	float3 r = reflect(lightDirection, normal);
	float rdotv = dot(r, normalize(toEye));
	specular = material.ReflectionIntesity.Specular * spotLight.Intensity.Specular * pow(max(rdotv, 0.0f), spotLight.Intensity.Specular.w); // max를 취해줘야 올바른 값이 나온다.

	// 중심의 거리와 지수에 따른 범위 제어
	float spot = pow(max(dot(lightDirection, spotLight.Direction), 0.0f), spotLight.Spot);

	// 감쇠율
	float invAttenuationRate = spot / dot(spotLight.AttenuationParam, float3(1.0f, distance, distance * distance));
	diffuse *= invAttenuationRate;
	specular *= invAttenuationRate;
}