#pragma once

#include <string>
#include <vector>
#include <map>

#include <directxtk/SimpleMath.h>

namespace resourceManager
{
	struct KeyAnimation
	{
		double Time;
		DirectX::SimpleMath::Vector3 Position;
		DirectX::SimpleMath::Quaternion Rotation;
		DirectX::SimpleMath::Vector3 Scaling;
	};

	struct AnimationNode
	{
	public:
		DirectX::SimpleMath::Matrix Evaluate(double progressTime) const;

	public:
		std::vector<KeyAnimation> KeyAnimations;
	};

	struct AnimationClip
	{
		double Duration;
		std::vector<AnimationNode> AnimationNodes;
	};
}