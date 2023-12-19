#pragma once

#include <vector>

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