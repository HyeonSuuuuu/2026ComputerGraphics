export module hs.shape;

import std;

export namespace hs
{
	struct Vec2 { float x{}, y{}; };
	struct Color { float r{ 1.f }, g{ 1.f }, b{ 1.f }; };

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
	};
}