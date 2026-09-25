export module hs.transform;

export import hs.bounds;

export namespace hs
{
	struct Transform
	{
		Vec2 pos;
		Vec2 size;

		Bounds GetBounds() const { return Bounds::FromCenterSize(pos, size); }
	};
}
