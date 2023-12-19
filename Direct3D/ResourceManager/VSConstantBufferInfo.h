#pragma once

#include "directxtk/SimpleMath.h"

struct VSConstantBufferInfo
{
	DirectX::SimpleMath::Matrix WorldTransform;
	DirectX::SimpleMath::Matrix ViewTransform;
	DirectX::SimpleMath::Matrix ProjectionTransform;
	DirectX::SimpleMath::Vector4 CameraPosition;
	DirectX::SimpleMath::Matrix BoneTransform[128];
};
