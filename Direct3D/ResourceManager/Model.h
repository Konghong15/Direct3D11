#pragma once

#include <vector>
#include <d3d11.h>

#include "Animation.h"

namespace resourceManager
{
	enum class eModelType
	{
		Normal,
		Skinned
	};

	struct Node;
	struct ModelDrawInfo;

	struct Model
	{
	public:
		void Release();
		void Draw(ID3D11DeviceContext* d3dImmediateContext, ModelDrawInfo& modelDrawInfo, double progressTime);

	public:
		Node* RootNode;
		Animation Animation;
		eModelType ModelType = eModelType::Normal;
	};
}