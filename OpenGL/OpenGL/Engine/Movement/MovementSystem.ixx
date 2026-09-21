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
	// Mover가 붙은 오브젝트를 매 프레임 움직인다
	// 상태를 갖지 않는다. 경계는 Scene이 들고 있다
	// 위치를 바꾸는 곳은 이 클래스 하나뿐이다
	class MovementSystem
	{
	public:
		static void Tick(Scene& scene, float dt)
		{
			Check(dt >= 0.f, "시간은 거꾸로 흐르지 않는다");

			const Bounds bounds = scene.GetBounds();
			for (Object& object : scene.Objects())
				if (object.mover && object.mover->enabled)
					Step(*object.mover, object.transform, dt, bounds);
		}

	private:
		static void Step(Mover& mover, Transform& transform, float dt, const Bounds& bounds)
		{
			// 1. 이동 방식이 속도를 정한다
			if (mover.mode)
				mover.mode->CalcVelocity(mover.velocity, transform, dt);

			// 2. 얹힌 LayeredMove들을 섞어 이번 프레임의 속도를 만든다
			Vec2 velocity = mover.velocity.Value();
			bool overridden = false;
			for (auto& move : mover.layeredMoves)
			{
				Vec2 value = move->Evaluate(transform, dt);
				if (move->GetMixMode() == MixMode::Override)
				{
					velocity = value;
					overridden = true;
				}
				else
				{
					velocity += value;
				}
			}
			std::erase_if(mover.layeredMoves, [](const auto& move) { return move->IsFinished(); });

			// NaN은 한 번 들어오면 계속 퍼져서 사각형이 화면에서 사라진다. 여기서 잡는다
			Check(std::isfinite(velocity.x) && std::isfinite(velocity.y), "속도가 NaN이나 무한이다");

			// 3. 적분. 위치를 바꾸는 유일한 지점
			transform.pos += velocity * dt;

			// 4. 경계 처리
			// Override 중에는 Reflect를 적용하지 않는다. LayeredMove가 정한 경로 때문에
			// 지속 속도의 방향이 뒤집히면, 그 이동이 끝난 뒤 엉뚱한 방향으로 움직인다
			ResolveBounds(transform, mover.velocity,
				overridden ? BoundsResponse::Clamp : mover.bounds, bounds);
		}

		// 축마다 같은 처리를 반복한다. 3D로 갈 때는 AxisCount만 3으로 바꾸면 된다
		static constexpr int AxisCount = 2;

		static void ResolveBounds(Transform& transform, Velocity& velocity, BoundsResponse response, const Bounds& bounds)
		{
			if (response == BoundsResponse::None)
				return;

			Vec2 half = transform.size / 2.f;

			for (int axis = 0; axis < AxisCount; ++axis)
			{
				if (response == BoundsResponse::Wrap)
				{
					// 완전히 빠져나간 뒤에 반대편으로 보낸다. 걸치자마자 옮기면 튀어 보인다
					if (transform.pos[axis] + half[axis] < bounds.min[axis])
						transform.pos[axis] = bounds.max[axis] + half[axis];
					else if (transform.pos[axis] - half[axis] > bounds.max[axis])
						transform.pos[axis] = bounds.min[axis] - half[axis];
					continue;
				}

				float low = bounds.min[axis] + half[axis];		// 중심이 가질 수 있는 범위
				float high = bounds.max[axis] - half[axis];
				if (low > high)									// 경계보다 큰 사각형은 밀어낼 곳이 없다
					continue;

				// Clamp와 Reflect는 되돌리는 처리가 같고, 방향 반전 여부만 다르다
				bool reflect = (response == BoundsResponse::Reflect);

				if (transform.pos[axis] < low)
				{
					transform.pos[axis] = low;
					// 부호를 뒤집는 대신 안쪽 방향으로 맞춘다. 한 프레임에 빠져나오지 못해도 끼지 않는다
					if (reflect) velocity.dir[axis] = std::abs(velocity.dir[axis]);
				}
				else if (transform.pos[axis] > high)
				{
					transform.pos[axis] = high;
					if (reflect) velocity.dir[axis] = -std::abs(velocity.dir[axis]);
				}
			}
		}
	};
}
