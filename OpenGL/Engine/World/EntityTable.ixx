export module hs.entity_table;

import std;
import hs.check;

export namespace hs
{
	// generation 0 = 빈 값. 소멸 후 같은 칸이 재사용돼도 세대가 달라 옛 ID로는 안 잡힘
	struct EntityId
	{
		std::uint32_t index{};
		std::uint32_t generation{};

		bool operator==(const EntityId&) const = default;
		explicit operator bool() const { return generation != 0; }
	};

	// 누가 살아 있고 언제 보이나. 생성은 Flush 뒤에 보이고, 소멸은 Flush에서 칸 반납
	class EntityTable
	{
	public:
		EntityId Create()
		{
			std::uint32_t index = Acquire();
			Slot& slot = _slots[index];
			slot.used = true;
			slot.alive = true;
			_pending.push_back(index);
			return { index, slot.generation };
		}

		void Destroy(EntityId id)
		{
			if (!IsAlive(id))
				return;				// 두 번 Destroy해도 목록엔 한 번만

			_slots[id.index].alive = false;
			_destroyed.push_back(id.index);
		}

		// 존재함 (Flush 전·Destroy 후도 true)
		bool IsValid(EntityId id) const
		{
			return id && id.index < _slots.size() && _slots[id.index].used && _slots[id.index].generation == id.generation;
		}

		bool IsAlive(EntityId id) const { return IsValid(id) && _slots[id.index].alive; }

		// 목록에 보임 (Flush를 거쳤고 Destroy 전)
		bool IsVisible(std::uint32_t index) const
		{
			const Slot& slot = _slots[index];
			return slot.used && slot.alive && slot.active;
		}

		EntityId IdAt(std::uint32_t index) const { return { index, _slots[index].generation }; }
		std::uint32_t SlotCount() const { return static_cast<std::uint32_t>(_slots.size()); }
		std::size_t VisibleCount() const { return _visibleCount; }

		// 생성분 노출, 소멸분 칸 반납. 반납하는 칸마다 onRemove(index)
		template<class F>
		void Flush(F&& onRemove)
		{
			for (std::uint32_t index : _pending)
			{
				Slot& slot = _slots[index];
				if (slot.used && slot.alive)
				{
					slot.active = true;
					++_visibleCount;
				}
			}
			_pending.clear();

			// 전 칸이 아니라 지운 것만 → 지운 게 없는 프레임은 비용 0
			for (std::uint32_t index : _destroyed)
			{
				Slot& slot = _slots[index];
				if (slot.active)
					--_visibleCount;

				onRemove(index);
				slot = { .generation = slot.generation + 1 };	// 이전 EntityId 무효화 지점
				_free.push_back(index);
			}
			_destroyed.clear();
		}

		void Clear()
		{
			_free.clear();
			for (std::uint32_t index = 0; index < _slots.size(); ++index)
			{
				Slot& slot = _slots[index];
				if (slot.used)
					slot = { .generation = slot.generation + 1 };
				_free.push_back(index);
			}
			_pending.clear();
			_destroyed.clear();
			_visibleCount = 0;
		}

	private:
		struct Slot
		{
			std::uint32_t generation{ 1 };	// 0 = 무효 EntityId → 1부터
			bool used{};					// 이 칸에 엔티티가 있음
			bool alive{};					// Destroy 전
			bool active{};					// Flush를 거쳐 목록에 보임
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
		std::vector<std::uint32_t> _pending;	// Flush에서 노출할 칸
		std::vector<std::uint32_t> _destroyed;	// Flush에서 반납할 칸
		std::size_t _visibleCount{};
	};
}
