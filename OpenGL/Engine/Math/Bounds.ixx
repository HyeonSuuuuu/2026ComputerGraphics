export module hs.bounds;

export import hs.vec2;

export namespace hs
{
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

		Vec2 Size() const { return max - min; }

		bool Contains(Vec2 p) const
		{
			return min.x <= p.x && p.x <= max.x
				&& min.y <= p.y && p.y <= max.y;
		}

		// 모서리 접촉은 겹침 아님
		bool Intersects(const Bounds& other) const
		{
			return min.x < other.max.x && other.min.x < max.x
				&& min.y < other.max.y && other.min.y < max.y;
		}
	};
}
