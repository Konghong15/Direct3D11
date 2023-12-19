#pragma once

#include <directxtk/SimpleMath.h>

struct Vertex
{
	enum { INVALID_INDEX = -1 };

	DirectX::SimpleMath::Vector4 Position;
	DirectX::SimpleMath::Vector3 Normal;
	DirectX::SimpleMath::Vector3 Tangent;
	DirectX::SimpleMath::Vector3 Binormal;
	DirectX::SimpleMath::Vector2 UV;
	int Indices[4] = { INVALID_INDEX, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX };
	float Weights[4] = { 0.f };
};