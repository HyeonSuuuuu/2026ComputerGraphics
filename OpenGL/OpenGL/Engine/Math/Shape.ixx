module;
#define GLM_FORCE_CTOR_INIT // 기본 생성자 0으로 초기화
#include <gl/glm/glm.hpp>

export module hs.shape;

import std;

export namespace hs
{
	using Vec2 = glm::vec2;
	
	float Dot(Vec2 a, Vec2 b) { return glm::dot(a, b); }
	float LengthSq(Vec2 v) { return glm::dot(v, v); }
	float Length(Vec2 v) { return glm::length(v); }
	Vec2 Lerp(Vec2 a, Vec2 b, float t) { return glm::mix(a, b, t); }
	
	Vec2 Normalize(Vec2 v)
	{
		return LengthSq(v) > 0.f ? glm::normalize(v) : Vec2{};
	}

	struct Color { float r{ 1.f }, g{ 1.f }, b{ 1.f }; };

	// hue는 0~1을 한 바퀴로 본다 (0 빨강 → 0.33 초록 → 0.66 파랑)
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

	// AABB
	struct Bounds
	{
		Vec2 min{};
		Vec2 max{};

		static Bounds FromCenterSize(Vec2 center, Vec2 size)
		{
			Vec2 half = size / 2.f;
			return { center - half, center + half };
		}

		Vec2 Center() const { return (min + max) / 2.f; }
		Vec2 Size() const { return max - min; }

		bool Contains(Vec2 p) const
		{
			return min.x <= p.x && p.x <= max.x
				&& min.y <= p.y && p.y <= max.y;
		}

		// other가 이 영역 안에 완전히 들어있는지
		bool Contains(const Bounds& other) const
		{
			return min.x <= other.min.x && other.max.x <= max.x
				&& min.y <= other.min.y && other.max.y <= max.y;
		}

		// 모서리만 맞닿은 경우는 겹침으로 보지 않음
		bool Intersects(const Bounds& other) const
		{
			return min.x < other.max.x && other.min.x < max.x
				&& min.y < other.max.y && other.min.y < max.y;
		}
	};
	
	struct Transform
	{
		Vec2 pos;
		Vec2 size;

		Bounds GetBounds() const { return Bounds::FromCenterSize(pos, size); }
	};
}