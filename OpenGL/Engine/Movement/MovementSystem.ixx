module;
#include "Core/Check.h"

export module hs.movement_system;

import std;
import hs.check;
import hs.transform;
import hs.movement;
export import hs.world;

export namespace hs
{
	// 상태 없음
	// 경계 복귀는 CollisionSystem 담당
	class MovementSystem
	{
	public:
		static void Tick(World& world, float dt)
		{
			HS_DCHECK(dt >= 0.f, "시간은 거꾸로 흐르지 않는다");

			world.Each<Mover, Transform>([dt](Entity, Mover& mover, Transform& transform)
				{
					if (mover.enabled)
						Step(mover, transform, dt);
				});
		}

	private:
		static void Step(Mover& mover, Transform& transform, float dt)
		{
			if (mover.mode)
				mover.mode->CalcVelocity(mover.velocity, transform, dt);

			Vec2 velocity = mover.velocity.Value();

			// NaN은 한 번 들어오면 전파 → 사각형 소실. 여기서 차단
			HS_DCHECK(std::isfinite(velocity.x) && std::isfinite(velocity.y), "속도가 NaN이나 무한이다");

			transform.pos += velocity * dt;
		}
	};
}
