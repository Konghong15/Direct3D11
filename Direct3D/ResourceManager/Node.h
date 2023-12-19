#pragma once

#include <map>

#include "Mesh.h"
#include "Animation.h"

namespace resourceManager
{
	struct ModelDrawInfo;

	struct Node
	{
	public:
		void Release();
		void UpdateRecursive(double progressTime, const Animation& animation);
		void DrawRecursive(ID3D11DeviceContext* d3dImmediateContext, ModelDrawInfo& modelDrawInfo);

	public:
		enum { INVALID_INDEX = -1 };

		Node* Parent = nullptr;
		std::vector<Node*> Children;
		std::string Name;
		DirectX::SimpleMath::Matrix BindPose = DirectX::SimpleMath::Matrix::Identity;
		DirectX::SimpleMath::Matrix LocalMatrix = DirectX::SimpleMath::Matrix::Identity;
		DirectX::SimpleMath::Matrix WorldMatrix = DirectX::SimpleMath::Matrix::Identity;
		std::vector<Mesh> Meshes;
		unsigned int AnimationNodeIndex = INVALID_INDEX;
	};
}