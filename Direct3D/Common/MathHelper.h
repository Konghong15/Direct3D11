#pragma once

#include <cmath>

namespace common
{
	class MathHelper
	{
	public:
		static float AngleFromXY(float x, float y);

	public:
		static constexpr float Pi = 3.1415926535f;
	};
}