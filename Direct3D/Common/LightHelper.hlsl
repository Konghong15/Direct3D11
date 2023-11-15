
struct LightElement
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
};

struct DirectionalLight
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

LightElement ComputeDirectionLight(Material material, DirectionalLight directionLight, float3 normal, float3 viewDirection)
{
	LightElement result;
	result.Ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	result.Diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	result.Specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	result.Ambient = material.ReflectionIntesity.Ambient * directionLight.Intensity.Ambient;

	float ndotl = dot(normal, -directionLight.Direction); // ������ ���� ���ݻ籤 ����
	
	if (ndotl < 0) // �븻�� ���� ������ �Ѿ ��� ���� ��ȯ
	{
		return result;
	}
	
	result.Diffuse = material.ReflectionIntesity.Diffuse * directionLight.Intensity.Diffuse * ndotl;
	// ���� ���ʹ� �׻� ����ȭ �Ǿ� �ִٰ� �����ؾ� �ϳ�? assert ���� �� �ֳ�?
	float rdotv = dot(reflect(directionLight.Direction, normal), normalize(viewDirection)); // ������ ���� ���ݻ籤 ����, ���� ������ ���� cos �׷����� ���� ���� ����Ѵ�.

	result.Specular = material.ReflectionIntesity.Specular * directionLight.Intensity.Specular * pow(rdotv, directionLight.Intensity.Specular.w);

	return result;
}

// ���� ����� ���� ��ȯ�̶� ������ ��� �κи� ���� direction�� �Ȱ��� ������ ȣ��� ó���ϴ� �� ��������
LightElement ComputePointLight(Material material, PointLight pointLight, float3 position, float3 normal, float3 viewDirection)
{
	LightElement result;
	result.Ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	result.Diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	result.Specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightDirection = position - pointLight.Position;
	float distance = length(lightDirection);
	
	if (distance > pointLight.Range)
	{
		return result;
	}

	result.Ambient = material.ReflectionIntesity.Ambient * pointLight.Intensity.Ambient;

	lightDirection /= distance;
	float ndotl = dot(normal, -lightDirection); // ������ ���� ���ݻ籤 ����
	
	if (ndotl < 0) // �븻�� ���� ������ �Ѿ ��� ���� ��ȯ
	{
		return result;
	}

	result.Diffuse = material.ReflectionIntesity.Diffuse * pointLight.Intensity.Diffuse * ndotl ;
	
	float rdotv = dot(reflect(lightDirection, normal), normalize(viewDirection));
	result.Specular = material.ReflectionIntesity.Specular * pointLight.Intensity.Specular * pow(rdotv, pointLight.Intensity.Specular.w);

	float invAttenuationRate = 1 / dot(pointLight.AttenuationParam, float3(1.0f, distance, distance * distance));
	result.Diffuse *= invAttenuationRate;
	result.Specular *= invAttenuationRate;

	return result;
}

LightElement ComputeSpotLight(Material material, SpotLight spotLight, float3 position, float3 normal, float3 viewDirection)
{
	LightElement result;
	result.Ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	result.Diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	result.Specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightDirection = position - spotLight.Position;
	float distance = length(lightDirection);

	if (distance > spotLight.Range)
	{
		return result;
	}

	result.Ambient = material.ReflectionIntesity.Ambient * spotLight.Intensity.Ambient;

	lightDirection /= distance;
	float ndotl = dot(normal, -lightDirection); // ������ ���� ���ݻ籤 ����
	
	if (ndotl < 0) // �븻�� ���� ������ �Ѿ ��� ���� ��ȯ
	{
		return result;
	}

	result.Diffuse = material.ReflectionIntesity.Diffuse * spotLight.Intensity.Diffuse * ndotl ;
	
	float rdotv = dot(reflect(lightDirection, normal), normalize(viewDirection));
	result.Specular = material.ReflectionIntesity.Specular * spotLight.Intensity.Specular * pow(rdotv, spotLight.Intensity.Specular.w);

	float spot = pow(max(dot(lightDirection, spotLight.Direction), 0.0f), spotLight.Spot);
	float invAttenuationRate = spot / dot(spotLight.AttenuationParam, float3(1.0f, distance, distance * distance));
	result.Diffuse *= invAttenuationRate;
	result.Specular *= invAttenuationRate;

	return result;
}