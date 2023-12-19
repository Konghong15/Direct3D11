#pragma once

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