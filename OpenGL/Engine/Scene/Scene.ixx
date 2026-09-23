export module hs.scene;

import std;
import hs.check;
import hs.transform;

export import :object;

export namespace hs
{
	// Objects(): 살아 있고 보이는 것만 → 시스템 쪽 필터 불필요
	// 소멸해도 슬롯 유지, 세대만 증가 → 예전 ObjectId 부활 없음
	class Scene
	{
	public:
		// 목록 변경은 Flush에서만 → 순회 중 호출 안전
		Object& Spawn(const Transform& transform)
		{
			Check(transform.size.x > 0.f && transform.size.y > 0.f, "크기가 0인 오브젝트는 보이지도 맞지도 않는다");

			std::uint32_t index = Acquire();
			Slot& slot = _slots[index];
			slot.object = std::make_unique<Object>(transform);
			slot.object->_id = { index, slot.generation };
			_pending.push_back(index);

			return *slot.object;
		}

		void Destroy(Object& object) { object._alive = false; }

		// 소멸 또는 세대 불일치(같은 자리 재사용)면 nullptr
		Object* Find(ObjectId id)
		{
			if (!id || id.index >= _slots.size())
				return nullptr;

			Slot& slot = _slots[id.index];
			if (!slot.object || !slot.active || !slot.object->IsAlive() || slot.generation != id.generation)
				return nullptr;

			return slot.object.get();
		}

		// 순회 중 호출 금지
		void Flush()
		{
			for (std::uint32_t index : _pending)
			{
				_slots[index].active = true;
				++_count;
			}
			_pending.clear();

			for (std::uint32_t index = 0; index < _slots.size(); ++index)
			{
				Slot& slot = _slots[index];
				if (!slot.object || slot.object->IsAlive())
					continue;

				if (slot.active)
					--_count;

				slot.object.reset();
				slot.active = false;
				++slot.generation;		// 이전 ObjectId 무효화 지점
				_free.push_back(index);
			}
		}

		void Clear()
		{
			for (Slot& slot : _slots)
				if (slot.object)
				{
					slot.object.reset();
					slot.active = false;
					++slot.generation;
				}

			_free.clear();
			_pending.clear();
			_count = 0;
			for (std::uint32_t index = 0; index < _slots.size(); ++index)
				_free.push_back(index);
		}

		// 레벨 변경 시에만 갱신
		void SetBounds(Bounds bounds)
		{
			Check(bounds.min.x <= bounds.max.x && bounds.min.y <= bounds.max.y, "bounds의 min과 max가 뒤집혀 있다");
			_bounds = bounds;
		}

		Bounds WorldBounds() const { return _bounds; }

		// auto 반환 → 클래스 안에서 쓰는 곳보다 먼저 정의 (GCC)
		auto Objects()
		{
			return _slots
				| std::views::filter([](const Slot& slot) { return slot.object && slot.active && slot.object->IsAlive(); })
				| std::views::transform([](Slot& slot) -> Object& { return *slot.object; });
		}

		auto Objects() const
		{
			return _slots
				| std::views::filter([](const Slot& slot) { return slot.object && slot.active && slot.object->IsAlive(); })
				| std::views::transform([](const Slot& slot) -> const Object& { return *slot.object; });
		}

		// 나중에 그린 것이 위 → 역순 탐색
		Object* HitTest(Vec2 point)
		{
			for (Object& object : Objects() | std::views::reverse)
				if (object.GetBounds().Contains(point))
					return &object;
			return nullptr;
		}

		Object* FindOverlap(const Object& target)
		{
			for (Object& object : Objects() | std::views::reverse)
				if (&object != &target && object.GetBounds().Intersects(target.GetBounds()))
					return &object;
			return nullptr;
		}

		std::size_t Size() const { return _count; }
		bool IsEmpty() const { return _count == 0; }

	private:
		struct Slot
		{
			std::unique_ptr<Object> object;
			std::uint32_t generation{ 1 };	// 0 = 무효 ObjectId → 1부터
			bool active{};				// Flush 전엔 비노출
		};

		std::uint32_t Acquire()
		{
			if (!_free.empty())
			{
				std::uint32_t index = _free.back();
				_free.pop_back();
				return index;
			}

			_slots.emplace_back();
			return static_cast<std::uint32_t>(_slots.size() - 1);
		}

		std::vector<Slot> _slots;
		std::vector<std::uint32_t> _free;
		std::vector<std::uint32_t> _pending;		// Flush에서 노출할 슬롯
		std::size_t _count{};				// 보이는 오브젝트 수
		Bounds _bounds{ .min{ -1.f, -1.f }, .max{ 1.f, 1.f } };	// 기본값: NDC 전체
	};
}
