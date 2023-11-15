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
		DirectX::SimpleMath::Vector3 AttenuationParam; // 감쇠 매개변수 a0, a1, a2
		float pad;
	};

	struct SpotLight
	{
		LightElement Intensity;
		DirectX::SimpleMath::Vector3 Direction;
		float Spot; // 원뿔에 사용될 지수
		DirectX::SimpleMath::Vector3 Position;
		float Range;
		DirectX::SimpleMath::Vector3 AttenuationParam; // 감쇠 매개변수 a0, a1, a2
		float pad;
	};

	struct Material
	{
		LightElement ReflectionIntesity; // specular의 마지막 성분을 지수로 사용함
		DirectX::SimpleMath::Vector4 Reflect;
	};
}