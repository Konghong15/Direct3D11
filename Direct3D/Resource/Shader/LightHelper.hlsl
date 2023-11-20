
struct DirectionLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float pad;
};

struct PointLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Position;
	float Range;
	float3 AttenuationParam;
	float pad;
};

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float Spot;
	float3 Position;
	float Range;
	float3 AttenuationParam;
	float pad;
};

struct Material
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular; // specular w = specular power
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

	// ������
	ambient = material.Ambient * directionLight.Ambient;

	// ���ݻ籤
	float ndotl = dot(normal, -directionLight.Direction);

	if (ndotl < 0)
	{
		return;
	}

	diffuse = material.Diffuse * directionLight.Diffuse * ndotl;

	// ���ݻ籤
	float3 r = reflect(directionLight.Direction, normal);
	float rdotv = dot(r, normalize(toEye));
	specular = material.Specular * directionLight.Specular * pow(max(rdotv, 0.0f), directionLight.Specular.w);
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

	// �ֺ���
	ambient = material.Ambient * pointLight.Ambient;

	// ���ݻ籤
	lightDirection /= distance;
	float ndotl = dot(normal, -lightDirection);

	if (ndotl < 0)
	{
		return;
	}

	diffuse = material.Diffuse * pointLight.Diffuse * ndotl;

	// ���ݻ籤
	float3 r = reflect(lightDirection, normal);
	float rdotv = dot(r, normalize(toEye));
	specular = material.Specular * pointLight.Specular * pow(max(rdotv, 0.0f), pointLight.Specular.w);

	// ������
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

	// �ֺ���
	ambient = material.Ambient * spotLight.Ambient;

	// ���ݻ籤
	lightDirection /= distance;
	float ndotl = dot(normal, -lightDirection);

	if (ndotl < 0)
	{
		return;
	}

	diffuse = material.Diffuse * spotLight.Diffuse * ndotl;

	// ���ݻ籤
	float3 r = reflect(lightDirection, normal);
	float rdotv = dot(r, normalize(toEye));
	specular = material.Specular * spotLight.Specular * pow(max(rdotv, 0.0f), spotLight.Specular.w); // max�� ������� �ùٸ� ���� ���´�.

	// �߽��� �Ÿ��� ������ ���� ���� ����
	float spot = pow(max(dot(lightDirection, spotLight.Direction), 0.0f), spotLight.Spot);

	// ������
	float invAttenuationRate = spot / dot(spotLight.AttenuationParam, float3(1.0f, distance, distance * distance));
	diffuse *= invAttenuationRate;
	specular *= invAttenuationRate;
}