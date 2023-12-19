#pragma once

#include <string>

#include <directxtk/SimpleMath.h>

namespace resourceManager
{
	struct Node;

	struct Bone
	{
		std::string Name;
		const Node* Node = nullptr;
		DirectX::SimpleMath::Matrix OffsetMatrix;
	};
}