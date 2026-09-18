module;
// 기본 생성자가 0으로 초기화되게 한다 (GLM 0.9.9부터는 기본이 미초기화)
#define GLM_FORCE_CTOR_INIT
#include <gl/glm/glm.hpp>

export module hs.shape;

import std;

export namespace hs
{
	using Vec2 = glm::vec2;

	// glm 함수를 hs 이름으로 감싼다. import hs.shape 만으로 쓸 수 있게 하기 위함
	float Dot(Vec2 a, Vec2 b) { return glm::dot(a, b); }
	float LengthSq(Vec2 v) { return glm::dot(v, v); }
	float Length(Vec2 v) { return glm::length(v); }
	Vec2 Lerp(Vec2 a, Vec2 b, float t) { return glm::mix(a, b, t); }

	// glm::normalize는 길이가 0이면 NaN을 돌려주므로 막아 둔다
	Vec2 Normalize(Vec2 v)
	{
		return LengthSq(v) > 0.f ? glm::normalize(v) : Vec2{};
	}

	struct Color { float r{ 1.f }, g{ 1.f }, b{ 1.f }; };

	// 축 정렬 경계 상자(AABB). 영역을 나타낼 때 쓴다
	// 3D로 갈 때는 Vec2를 Vec3로 바꾸면 그대로 동작한다
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
	};

	struct Rectangle
	{
		Vec2 pos;
		Vec2 size;
		Color color;

		float Left() const { return pos.x - size.x / 2; }
		float Right() const { return pos.x + size.x / 2; }
		float Bottom() const { return pos.y - size.y / 2; }
		float Top() const { return pos.y + size.y / 2; }

		bool Contains(Vec2 p) const
		{
			return Left() <= p.x && p.x <= Right()
				&& Bottom() <= p.y && p.y <= Top();
		}

		// other가 이 사각형 안에 완전히 들어있는지
		bool Contains(const Rectangle& other) const
		{
			return Left() <= other.Left() && other.Right() <= Right()
				&& Bottom() <= other.Bottom() && other.Top() <= Top();
		}

		// 모서리만 맞닿은 경우는 겹침으로 보지 않음
		bool Intersects(const Rectangle& other) const
		{
			return Left() < other.Right() && other.Left() < Right()
				&& Bottom() < other.Top() && other.Bottom() < Top();
		}

		Bounds GetBounds() const { return Bounds::FromCenterSize(pos, size); }
	};
}