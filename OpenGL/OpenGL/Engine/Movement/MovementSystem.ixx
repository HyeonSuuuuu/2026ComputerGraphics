module;
#include <gl/glm/glm.hpp>

export module hs.movement_system;

import std;
import hs.check;
import hs.shape;
import hs.movement;
import hs.scene;

export namespace hs
{
	// Mover가 붙은 오브젝트를 매 프레임 움직인다. 상태를 갖지 않는다
	// 경계 밖으로 나가는 것은 CollisionSystem이 되돌린다
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

			// NaN은 한 번 들어오면 계속 퍼져서 사각형이 화면에서 사라진다. 여기서 잡는다
			Check(std::isfinite(velocity.x) && std::isfinite(velocity.y), "속도가 NaN이나 무한이다");

			transform.pos += velocity * dt;
		}
	};
}
