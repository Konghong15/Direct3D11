#pragma once

#include <vector>
#include <string>
#include <map>

#include <d3d11.h>
#include <directxtk/SimpleMath.h>

namespace resourceManager
{
	enum class eTextureBitFlag
	{
		Diffuse,
		Normals,
		Specular,
		Opacity,
		Size
	};

	union TextureBitFlag
	{
		unsigned int Value;
		struct
		{
			unsigned char BHasDiffuseMap : 1;
			unsigned char BHasNormalMap : 1;
			unsigned char BHasSpecularMap : 1;
			unsigned char BHasOpacityMap : 1;
		};
	};

	struct Vertex
	{
		DirectX::SimpleMath::Vector4 Position;
		DirectX::SimpleMath::Vector3 Normal;
		DirectX::SimpleMath::Vector3 Tangent;
		DirectX::SimpleMath::Vector3 Binormal;
		DirectX::SimpleMath::Vector2 UV;
	};

	struct VSConstantBufferInfo
	{
		DirectX::SimpleMath::Matrix WorldTransform;
		DirectX::SimpleMath::Matrix WVPTransform;
		DirectX::SimpleMath::Vector4 CameraPosition;
	};

	struct PSConstantBufferInfo
	{
		union LightBitFlag
		{
			unsigned int Value;
			struct
			{
				unsigned char BIsAppliedAmbient : 1;
				unsigned char BIsAppliedDiffuse : 1;
				unsigned char BIsAppliedSpecular : 1;
				unsigned char BIsAppliedNormalMap : 1;
				unsigned char BIsAppliedSpecularMap : 1;
			};
		};

		DirectX::SimpleMath::Vector4 LightDirection;
		DirectX::SimpleMath::Vector4 LightColor;
		unsigned int SpecularPow;
		LightBitFlag LightBitFlag;
		TextureBitFlag TextureBitFlag;
		unsigned int unused[1];
	};
}