export module hs.color;

import std;
import hs.random;

export namespace hs
{
	struct Color { float r{ 1.f }, g{ 1.f }, b{ 1.f }; };

	Color RandomColor()
	{
		return { Random(0.f, 1.f), Random(0.f, 1.f), Random(0.f, 1.f) };
	}

	// hue 0~1이 한 바퀴 (0 빨강, 0.33 초록, 0.66 파랑)
	Color FromHsv(float hue, float saturation = 1.f, float value = 1.f)
	{
		hue = hue - std::floor(hue);

		float sector = hue * 6.f;
		float f = sector - std::floor(sector);
		float p = value * (1.f - saturation);
		float q = value * (1.f - f * saturation);
		float t = value * (1.f - (1.f - f) * saturation);

		switch (static_cast<int>(sector) % 6)
		{
		case 0:  return { value, t, p };
		case 1:  return { q, value, p };
		case 2:  return { p, value, t };
		case 3:  return { p, q, value };
		case 4:  return { t, p, value };
		default: return { value, p, q };
		}
	}
}
