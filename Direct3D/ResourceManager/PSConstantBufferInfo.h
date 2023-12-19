#pragma once

#include <directxtk/SimpleMath.h>

#include "LightBitFlag.h"
#include "TextureBitFlag.h"

struct PSConstantBufferInfo
{
	DirectX::SimpleMath::Vector4 LightDirection;
	DirectX::SimpleMath::Vector4 LightColor;
	unsigned int SpecularPow;
	LightBitFlag LightBitFlag;
	TextureBitFlag TextureBitFlag;
	unsigned int unused[1];
};