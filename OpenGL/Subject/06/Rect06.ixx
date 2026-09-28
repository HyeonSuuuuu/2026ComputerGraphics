export module app06.rect;

import std;
import hs;

using namespace hs;

export namespace app06
{
    class Rect
    {
    public:
        // 처음 흩어 놓는 사각형. 멈춘 Mover는 Separate에 밀려도 화면 안에 붙잡아 두는 용도
        static Entity Spawn(World& world, Vec2 pos, Vec2 size)
        {
            Entity entity = world.Spawn({ .pos = pos, .size = size });
            entity.Add<Visual>(RandomColor());
            entity.Add<Mover>(Velocity{}, BoundsResponse::Clamp);
            return entity;
        }

        // 쪼개진 조각: goal까지 가서 멈추고, 색은 한 단계 어둡게
        static Entity SpawnPiece(World& world, const Transform& transform, Color color, Vec2 goal)
        {
            Entity entity = world.Spawn(transform);
            entity.Add<Visual>(color);
            entity.Add<Mover>(Velocity{ .speed = Speed }, BoundsResponse::Clamp)
                .SetMode(std::make_unique<FollowMode>(goal));
            entity.Add<Trigger>();	// 밀어내기가 도착 지점을 흐트러뜨림
            entity.Add<EffectStack>().Add<ColorFade>(color, StepToward(color, Black), ColorDuration);
            return entity;
        }

        static constexpr float Speed = 0.5f;
        static constexpr float MoveDistance = 0.2f;
        static constexpr float MinSize = 0.1f;		// 이보다 작으면 클릭 시 쪼개지지 않고 사라짐

    private:
        // 채널마다 최대 ColorStep씩: (255,0,0) → (204,0,0) → (153,0,0) → … → (0,0,0)
        static Color StepToward(Color from, Color to)
        {
            auto step = [](float a, float b) { return a + std::clamp(b - a, -ColorStep, ColorStep); };
            return { step(from.r, to.r), step(from.g, to.g), step(from.b, to.b) };
        }

        static constexpr Color Black{ 0.f, 0.f, 0.f };
        static constexpr float ColorStep = 0.2f;
        static constexpr float ColorDuration = MoveDistance / Speed;	// 이동과 같이 끝남
    };
}
