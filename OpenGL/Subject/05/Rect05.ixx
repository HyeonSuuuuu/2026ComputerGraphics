export module app05.rect;

import std;
import hs;

using namespace hs;

export namespace app05
{
    class Rect
    {
    public:
        static Entity Spawn(World& world, Vec2 pos)
        {
            Entity entity = world.Spawn({ .pos = pos, .size = { Size, Size } });
            entity.Add<Visual>(RandomColor());
            return entity;
        }

        static constexpr float Size = 0.12f;
        static constexpr float Speed = 0.5f;
    };
}
