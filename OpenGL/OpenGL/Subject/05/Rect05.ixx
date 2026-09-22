export module app05.rect;

import std;
import hs;

export namespace hs
{
    class Rect
    {
    public:
        static Object& Spawn(Scene& scene, Vec2 pos)
        {
            Object& object = scene.Spawn({ .pos = pos, .size = { Size, Size } });
            object.Add<Visual>(RandomColor());
            return object;
        }

        static constexpr float Size = 0.12f;
        static constexpr float Speed = 0.5f;
    };
}
