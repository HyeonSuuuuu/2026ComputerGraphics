export module hs.collision_system;

import std;
import hs.transform;
import hs.movement;
export import hs.scene;

export namespace hs
{
	// 밀어내기 없이 통지만
	struct Trigger { };

	// a: index가 작은 쪽
	struct Contact
	{
		ObjectId a;
		ObjectId b;
	};

	// 모든 값 확정 뒤, 프레임 마지막
	class CollisionSystem
	{
	public:
		// 조회만. 변경 없음
		static void FindContacts(Scene& scene, std::vector<Contact>& contacts)
		{
			ForEachPair(scene, [&contacts](Object a, Object b)
				{
					if (a.GetBounds().Intersects(b.GetBounds()))
						contacts.push_back({ a.Id(), b.Id() });
				});
		}

		static void Tick(Scene& scene)
		{
			const Bounds bounds = scene.WorldBounds();
			scene.Each<Mover, Transform>([bounds](Object, Mover& mover, Transform& transform)
				{
					ResolveBounds(transform, mover.velocity, mover.boundsResponse, bounds);
				});
		}

		// 한 번에 다 밀면 떨림 → 조금씩
		static void Separate(Scene& scene)
		{
			ForEachPair(scene, [](Object a, Object b)
				{
					if (a.Get<Trigger>() || b.Get<Trigger>())
						return;

					Transform& ta = a.GetTransform();
					Transform& tb = b.GetTransform();
					Vec2 delta = tb.pos - ta.pos;
					Vec2 overlap = (ta.size + tb.size) / 2.f + Vec2(SeparateGap) - Abs(delta);
					if (overlap.x <= 0.f || overlap.y <= 0.f)
						return;

					// 덜 파고든 축만: 모서리에서 옆으로 튐 방지
					Vec2 push{};
					if (overlap.x < overlap.y)
						push.x = delta.x < 0.f ? -overlap.x : overlap.x;
					else
						push.y = delta.y < 0.f ? -overlap.y : overlap.y;

					push *= SeparateStrength * 0.5f;	// 절반씩
					ta.pos -= push;
					tb.pos += push;
				});
		}

	private:
		// 순회 방법은 이 한 곳에만. 칸 순서(Objects)라 a가 항상 index가 작은 쪽
		// 전부 대 전부. 수백 개 넘으면 여기만 공간 분할로
		template<class F>
		static void ForEachPair(Scene& scene, F&& body)
		{
			auto objects = scene.Objects();
			for (auto it = objects.begin(); it != objects.end(); ++it)
				for (auto other = std::next(it); other != objects.end(); ++other)
					body(*it, *other);
		}

		// 프레임당 겹침 해소 비율 (1: 즉시)
		static constexpr float SeparateStrength = 0.05f;
		
		static constexpr float SeparateGap = 0.02f;

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
					// 완전히 빠져나간 뒤 이동. 걸치자마자 옮기면 튐
					if (transform.pos[axis] + half[axis] < bounds.min[axis])
						transform.pos[axis] = bounds.max[axis] + half[axis];
					else if (transform.pos[axis] - half[axis] > bounds.max[axis])
						transform.pos[axis] = bounds.min[axis] - half[axis];
					continue;
				}

				float low = bounds.min[axis] + half[axis];		// 중심의 허용 범위
				float high = bounds.max[axis] - half[axis];
				if (low > high)									// 경계보다 큰 사각형: 밀 곳 없음
					continue;

				bool reflect = (response == BoundsResponse::Reflect);

				if (transform.pos[axis] < low)
				{
					transform.pos[axis] = low;
					// 부호 반전 대신 안쪽 고정: 한 프레임에 못 빠져나와도 끼임 없음
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
