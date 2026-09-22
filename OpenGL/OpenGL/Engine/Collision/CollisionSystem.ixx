module;
#include <gl/glm/glm.hpp>

export module hs.collision_system;

import std;
import hs.check;
import hs.shape;
import hs.movement;
import hs.scene;

export namespace hs
{
	// 겹친 두 오브젝트. 무엇을 할지는 게임이 정한다
	struct Hit
	{
		Object* a;
		Object* b;
	};

	// 겹침을 해소한다. 모든 값이 정해진 뒤 프레임 마지막에 돈다
	// 지금은 경계(벽)만 본다. 오브젝트끼리는 여기에 붙는다
	class CollisionSystem
	{
	public:
		// 겹친 쌍을 모은다. 아무것도 바꾸지 않는다
		static void FindHits(Scene& scene, std::vector<Hit>& hits)
		{
			// 오브젝트가 적어 전부 대 전부로 본다. 수백 개가 넘으면 공간 분할이 필요하다
			auto objects = scene.Objects();
			for (auto it = objects.begin(); it != objects.end(); ++it)
			{
				Object& a = *it;
				for (auto other = std::next(it); other != objects.end(); ++other)
				{
					Object& b = *other;
					if (a.GetBounds().Intersects(b.GetBounds()))
						hits.push_back({ &a, &b });
				}
			}
		}

		// 경계 밖으로 나간 것을 되돌린다
		static void Tick(Scene& scene)
		{
			const Bounds bounds = scene.WorldBounds();
			for (Object& object : scene.Objects())
			{
				Mover* mover = object.Get<Mover>();
				if (!mover)
					continue;

				ResolveBounds(object.transform, mover->velocity, mover->boundsResponse, bounds);
			}
		}

	private:
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
