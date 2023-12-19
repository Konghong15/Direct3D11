#pragma once

#include <string>

#include <directxtk/SimpleMath.h>

struct Node;

struct Bone
{
	std::string Name;
	const Node* Node = nullptr;
	DirectX::SimpleMath::Matrix OffsetMatrix;
};