module;
#include <glm/glm.hpp>

export module hs.collision_system;

import std;
import hs.check;
import hs.shape;
import hs.component;
import hs.movement;
import hs.scene;

export namespace hs
{
	// 밀어내기 없이 통지만
	struct Trigger : IComponent { };
	

	// a: index가 작은 쪽
	struct Contact
	{
		ObjectId a;
		ObjectId b;
	};

	enum class HitPhase { Enter, Stay, Exit };

	// Exit 시점엔 한쪽이 이미 소멸했을 수 있음
	struct Hit
	{
		ObjectId a;
		ObjectId b;
		HitPhase phase;
	};

	// 모든 값 확정 뒤, 프레임 마지막
	class CollisionSystem
	{
	public:
		// 조회만. 변경 없음
		static void FindContacts(Scene& scene, std::vector<Contact>& contacts)
		{
			ForEachPair(scene, [&contacts](Object& a, Object& b)
				{
					if (a.GetBounds().Intersects(b.GetBounds()))
						contacts.push_back({ a.Id(), b.Id() });
				});
		}

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

		// 한 번에 다 밀면 떨림 → 조금씩
		static void Separate(Scene& scene)
		{
			ForEachPair(scene, [](Object& a, Object& b)
				{
					if (a.Get<Trigger>() || b.Get<Trigger>())
						return;

					Vec2 delta = b.transform.pos - a.transform.pos;
					Vec2 overlap = (a.transform.size + b.transform.size) / 2.f + Vec2(SeparateGap) - glm::abs(delta);
					if (overlap.x <= 0.f || overlap.y <= 0.f)
						return;

					// 덜 파고든 축만: 모서리에서 옆으로 튐 방지
					Vec2 push{};
					if (overlap.x < overlap.y)
						push.x = delta.x < 0.f ? -overlap.x : overlap.x;
					else
						push.y = delta.y < 0.f ? -overlap.y : overlap.y;

					push *= SeparateStrength * 0.5f;	// 절반씩
					a.transform.pos -= push;
					b.transform.pos += push;
				});
		}

	private:
		// 순회 방법은 이 한 곳에만
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

	// 상태 보유 → 시스템이 아닌 값. 쓰는 쪽이 하나 소유
	class ContactTracker
	{
	public:
		const std::vector<Hit>& Update(Scene& scene)
		{
			_current.clear();
			CollisionSystem::FindContacts(scene, _current);
			std::ranges::sort(_current, Less);

			// 정렬된 두 목록 병합: 이번만 Enter, 양쪽 Stay, 지난번만 Exit
			_hits.clear();
			std::size_t now = 0;
			std::size_t before = 0;
			while (now < _current.size() || before < _previous.size())
			{
				if (before == _previous.size() || (now < _current.size() && Less(_current[now], _previous[before])))
					_hits.push_back({ _current[now].a, _current[now].b, HitPhase::Enter }), ++now;
				else if (now == _current.size() || Less(_previous[before], _current[now]))
					_hits.push_back({ _previous[before].a, _previous[before].b, HitPhase::Exit }), ++before;
				else
					_hits.push_back({ _current[now].a, _current[now].b, HitPhase::Stay }), ++now, ++before;
			}

			_previous.swap(_current);
			return _hits;
		}

		// 다음 판정에서 Enter 대신 Stay
		// 겹친 자리에서 생성된 오브젝트의 즉시 처리 방지용
		void AssumeTouching(ObjectId a, ObjectId b)
		{
			const Contact contact = LessId(a, b) ? Contact{ a, b } : Contact{ b, a };

			const auto pos = std::ranges::lower_bound(_previous, contact, Less);
			if (pos == _previous.end() || Less(contact, *pos))
				_previous.insert(pos, contact);
		}

		void Clear()
		{
			_previous.clear();
			_hits.clear();
		}

	private:
		static bool LessId(ObjectId x, ObjectId y)
		{
			return std::tie(x.index, x.generation) < std::tie(y.index, y.generation);
		}

		static bool Less(const Contact& x, const Contact& y)
		{
			return std::tie(x.a.index, x.a.generation, x.b.index, x.b.generation)
				 < std::tie(y.a.index, y.a.generation, y.b.index, y.b.generation);
		}

		std::vector<Contact> _current;
		std::vector<Contact> _previous;
		std::vector<Hit> _hits;
	};
}
