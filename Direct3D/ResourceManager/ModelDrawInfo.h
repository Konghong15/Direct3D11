#pragma once

#include <d3d11.h>

#include "VSConstantBufferInfo.h"
#include "PSConstantBufferInfo.h"

namespace resourceManager
{
	struct ModelDrawInfo
	{
		ID3D11Buffer* VSConstantBuffer;
		ID3D11Buffer* PSConstantBuffer;
		VSConstantBufferInfo VSConstantBufferInfo;
		PSConstantBufferInfo PSConstantBufferInfo;
	};
}