export module hs.scene;

import std;
import hs.check;
import hs.shape;

export import hs.object; // 오브젝트에 의존함, 쓰는 쪽이 편하게 함께 내보냄

export namespace hs
{
	// Objects()는 살아 있고 이미 보이는 것만 준다. 시스템은 따로 거르지 않아도 된다
	// 오브젝트를 슬롯에 담는다. 죽어도 슬롯은 남고 세대만 오르므로, 예전 ObjectId가 되살아나지 않는다
	class Scene
	{
	public:
		// 목록은 Flush에서만 바뀐다. 순회 중 어디서 불러도 안전하다
		Object& Spawn(const Transform& transform)
		{
			Check(transform.size.x > 0.f && transform.size.y > 0.f, "크기가 0인 오브젝트는 보이지도 맞지도 않는다");

			std::uint32_t index = Acquire();
			Slot& slot = _slots[index];
			slot.object = std::make_unique<Object>(transform);
			slot.object->_id = { index, slot.generation };
			_pending.push_back(index);	// Flush에서 보이기 시작한다

			return *slot.object;
		}

		void Destroy(Object& object) { object._alive = false; }

		// 죽었으면 nullptr. 세대가 달라도 nullptr (같은 자리에 다른 오브젝트가 들어온 경우)
		Object* Find(ObjectId id)
		{
			if (!id || id.index >= _slots.size())
				return nullptr;

			Slot& slot = _slots[id.index];
			if (!slot.object || !slot.active || slot.generation != id.generation)
				return nullptr;

			return slot.object.get();
		}

		// 프레임의 정해진 한 지점에서 부른다. 순회 중에 부르면 안 된다
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
				++slot.generation;		// 이전 ObjectId는 여기서 무효가 된다
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

		// 오브젝트가 돌아다닐 수 있는 영역. 레벨이 바뀔 때만 갱신하면 된다
		void SetBounds(Bounds bounds)
		{
			Check(bounds.min.x <= bounds.max.x && bounds.min.y <= bounds.max.y, "bounds의 min과 max가 뒤집혀 있다");
			_bounds = bounds;
		}

		Bounds WorldBounds() const { return _bounds; }

		// 나중에 그려진 것이 위에 있으므로 뒤에서부터 찾는다
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

		std::size_t Size() const { return _count; }
		bool IsEmpty() const { return _count == 0; }

	private:
		struct Slot
		{
			std::unique_ptr<Object> object;
			std::uint32_t generation{ 1 };	// 0은 무효한 ObjectId를 뜻하므로 1부터
			bool active{};				// Flush 전에는 아직 안 보인다
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
		std::vector<std::uint32_t> _free;		// 비어 있는 슬롯
		std::vector<std::uint32_t> _pending;		// Flush에서 보이게 할 슬롯
		std::size_t _count{};				// 보이는 오브젝트 수
		Bounds _bounds{ .min{ -1.f, -1.f }, .max{ 1.f, 1.f } };	// 기본값은 NDC 전체
	};
}
