module;
#include "Core/Check.h"

export module hs.random;

import std;
import hs.check;
export import hs.vec2;

namespace hs
{
	std::mt19937 gen(std::random_device{}());
}

export namespace hs
{
	template<std::integral T>
	T Random(T min, T max)
	{
		HS_DCHECK(min <= max, "최소가 최대보다 큼");
		return std::uniform_int_distribution<T>(min, max)(gen);
	}

	template<std::floating_point T>
	T Random(T min, T max)
	{
		HS_DCHECK(min <= max, "최소가 최대보다 큼");
		return std::uniform_real_distribution<T>(min, max)(gen);
	}
	
	
	Vec2 RandomVec2(const float min, const float max)
	{
		return { Random(min, max), Random(min, max) };
	}

	Vec2 RandomVec2(const Vec2& min, const Vec2& max)
	{
		return { Random(min.x, max.x), Random(min.y, max.y) };
	}
}