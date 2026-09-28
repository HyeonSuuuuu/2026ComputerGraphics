export module hs.easing;

import std;

export namespace hs
{
	using EaseFn = std::function<float(float)>;

	float Linear(float t) { return t; }

	float EaseIn(float t) { return t * t; }	
	float EaseOut(float t) { return 1 - (1 - t) * (1 - t); }
	float EaseInOut(float t)
	{
		float u = -2 * t + 2;
		return t < 0.5f ? 2 * t * t : 1 - u * u / 2;
	}

	float EaseInCubic(float t) { return t * t * t; }
	float EaseOutCubic(float t) { return 1 - (1 - t) * (1 - t) * (1 - t); }
	
	float EaseOutBack(float t)
	{
		constexpr float c1 = 1.70158f;
		constexpr float c3 = c1 + 1;
		float u = t - 1;
		return 1 + c3 * u * u * u + c1 * u * u;
	}
	
	float PingPong(float t) { return t < 0.5f ? t * 2 : 2 - t * 2; }

	float Reverse(float t) { return 1 - t; }

	EaseFn Compose(EaseFn f, EaseFn g)
	{
		return [f = std::move(f), g = std::move(g)](float t) { return g(f(t)); };
	}
}
