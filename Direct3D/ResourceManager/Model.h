#pragma once

#include <vector>
#include <d3d11.h>

#include "Animation.h"
#include "Node.h"

enum class eModelType
{
	Normal,
	Skinned
};

struct Model
{
	Node* RootNode;
	Animation Animation;
	eModelType ModelType = eModelType::Normal;

	void Release()
	{
		RootNode->Release();
		delete RootNode;
		RootNode = nullptr;
	}

	void Draw(ID3D11DeviceContext* d3dImmediateContext, ModelDrawInfo& modelDrawInfo, double progressTime)
	{
		RootNode->UpdateRecursive(progressTime, Animation);
		RootNode->DrawRecursive(d3dImmediateContext, modelDrawInfo);
	}
};
