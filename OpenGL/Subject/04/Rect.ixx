export module app04.rect;

import std;
import hs;

using namespace hs;

export namespace app04
{
	// 복귀 지점 = 처음 생성 위치
	struct Home
	{
		explicit Home(Vec2 pos) : pos(pos) {}

		Vec2 pos;
	};

	class Rect
	{
	public:
		static Entity Spawn(World& world, Vec2 pos)
		{
			Entity entity = world.Spawn({ .pos = pos, .size = { Size, Size } });
			entity.Add<Home>(pos);
			entity.Add<Visual>(RandomColor());
			entity.Add<Mover>(Velocity{ .dir = RandomDiagonal(), .speed = Speed });
			EffectStack& effects = entity.Add<EffectStack>();
			effects.Add<ScalePulse>();
			effects.Add<ColorCycle>(3.f, Random(0.f, 1.f));	// 시작 색 분산
			
			return entity;
		}

		static Vec2 RandomDiagonal()
		{
			constexpr float d = 0.70710678f;	// 1/√2
			return { Random(0, 1) ? d : -d, Random(0, 1) ? d : -d };
		}

		static constexpr float Size = 0.12f;
		static constexpr float Speed = 0.5f;
	};
}
