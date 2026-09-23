module;
#include <glm/glm.hpp>

export module hs.movement_system;

import std;
import hs.check;
import hs.shape;
import hs.movement;
import hs.scene;

export namespace hs
{
	// 상태 없음
	// 경계 복귀는 CollisionSystem 담당
	class MovementSystem
	{
	public:
		static void Tick(Scene& scene, float dt)
		{
			Check(dt >= 0.f, "시간은 거꾸로 흐르지 않는다");

			for (Object& object : scene.Objects())
			{
				Mover* mover = object.Get<Mover>();
				if (mover && mover->enabled)
					Step(*mover, object.transform, dt);
			}
		}

	private:
		static void Step(Mover& mover, Transform& transform, float dt)
		{
			if (mover.mode)
				mover.mode->CalcVelocity(mover.velocity, transform, dt);

			Vec2 velocity = mover.velocity.Value();

			// NaN은 한 번 들어오면 전파 → 사각형 소실. 여기서 차단
			Check(std::isfinite(velocity.x) && std::isfinite(velocity.y), "속도가 NaN이나 무한이다");

			transform.pos += velocity * dt;
		}
	};
}
