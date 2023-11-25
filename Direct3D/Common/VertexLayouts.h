#pragma once

#include <d3d11.h>
#include <directxtk/SimpleMath.h>

namespace common
{
	struct Basic32
	{
		DirectX::SimpleMath::Vector3 Pos;
		DirectX::SimpleMath::Vector3 Normal;
		DirectX::SimpleMath::Vector3 Tex;
	};

	struct PointSprite
	{
		DirectX::SimpleMath::Vector3 Pos;
		DirectX::SimpleMath::Vector2 Size;
	};

	class VertexLayouts
	{
	public:
		static void Init(ID3D11Device* device);
		static void Destroy();

	public:
		static ID3D11InputLayout* Basic32;
		static ID3D11InputLayout* TreePointSprite;
	};
}