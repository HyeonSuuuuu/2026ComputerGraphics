export module hs.contact_tracker;

import std;
export import hs.collision_system;

export namespace hs
{
	enum class HitPhase { Enter, Stay, Exit };

	// Exit 시점엔 한쪽이 이미 소멸했을 수 있음
	struct Hit
	{
		ObjectId a;
		ObjectId b;
		HitPhase phase;
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
