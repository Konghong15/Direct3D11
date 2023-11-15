#pragma once

#include <directxtk/SimpleMath.h>

namespace common
{
	struct LightElement
	{
		DirectX::SimpleMath::Vector4 Ambient;
		DirectX::SimpleMath::Vector4 Diffuse;
		DirectX::SimpleMath::Vector4 Specular;
	};

	struct DirectionLight
	{
		LightElement Intensity;
		DirectX::SimpleMath::Vector3 Direction;
		float pad;
	};

	struct PointLight
	{
		LightElement Intensity;
		DirectX::SimpleMath::Vector3 Position;
		float Range;
		DirectX::SimpleMath::Vector3 AttenuationParam; // ���� �Ű����� a0, a1, a2
		float pad;
	};

	struct SpotLight
	{
		LightElement Intensity;
		DirectX::SimpleMath::Vector3 Direction;
		float Spot; // ���Կ� ���� ����
		DirectX::SimpleMath::Vector3 Position;
		float Range;
		DirectX::SimpleMath::Vector3 AttenuationParam; // ���� �Ű����� a0, a1, a2
		float pad;
	};

	struct Material
	{
		LightElement ReflectionIntesity; // specular�� ������ ������ ������ �����
		DirectX::SimpleMath::Vector4 Reflect;
	};
}