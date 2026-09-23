export module hs.random;

import std;
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
		return std::uniform_int_distribution<T>(min, max)(gen);
	}

	template<std::floating_point T>
	T Random(T min, T max)
	{
		return std::uniform_real_distribution<T>(min, max)(gen);
	}

	Vec2 RandomVec2(const Vec2& min, const Vec2& max)
	{
		return { Random(min.x, max.x), Random(min.y, max.y) };
	}
}