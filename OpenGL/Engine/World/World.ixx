export module hs.world;

import std;
import hs.check;
export import hs.transform;
export import hs.entity_table;
import hs.component_pools;

export namespace hs
{
	class World;

	// 컴포넌트를 들고 있지 않은 번호표(EntityId + World). 값으로 복사해 넘김
	// Get으로 받은 포인터는 그 자리에서만: 같은 타입의 Add·삭제가 저장소를 재배치함
	class Entity
	{
	public:
		Entity() = default;

		// 존재하는 엔티티를 가리키는지 (Flush 전·Destroy 후도 true. 목록 노출 여부는 World::Find)
		explicit operator bool() const;
		bool operator==(const Entity&) const = default;

		EntityId Id() const { return _id; }
		bool IsAlive() const;

		template<class T, class... Args>
		T& Add(Args&&... args) const;

		template<class T>
		T* Get() const;

		template<class T>
		bool Remove() const;

		Transform& GetTransform() const { return *Get<Transform>(); }
		Bounds GetBounds() const { return GetTransform().GetBounds(); }

	private:
		friend class World;
		Entity(EntityId id, World* world) : _id(id), _world(world) {}

		EntityId _id;
		World* _world{};
	};

	// 게임 한 판의 전부. 누가 있나(EntityTable) + 무엇을 들었나(ComponentPools) + 조회
	class World
	{
	public:
		World() = default;
		World(const World&) = delete;				// Entity가 World 주소를 들고 있음
		World& operator=(const World&) = delete;

		Entity Spawn(const Transform& transform)
		{
			Check(transform.size.x > 0.f && transform.size.y > 0.f, "크기가 0인 엔티티는 보이지도 맞지도 않는다");

			Entity entity{ _entities.Create(), this };
			entity.Add<Transform>(transform);
			return entity;
		}

		void Destroy(Entity entity) { _entities.Destroy(entity._id); }

		// 목록에 보이는 것만. 아니면 빈 Entity
		Entity Find(EntityId id)
		{
			return _entities.IsValid(id) && _entities.IsVisible(id.index) ? Entity{ id, this } : Entity{};
		}

		// 순회 중 호출 금지
		void Flush()
		{
			Check(_iterating == 0, "Each 도중 Flush 금지");
			_entities.Flush([this](std::uint32_t index) { _components.RemoveAll(index); });
		}

		void Clear()
		{
			Check(_iterating == 0, "Each 도중 Clear 금지");
			_components.Clear();
			_entities.Clear();
		}

		// 레벨 변경 시에만 갱신
		void SetBounds(Bounds bounds)
		{
			Check(bounds.min.x <= bounds.max.x && bounds.min.y <= bounds.max.y, "bounds의 min과 max가 뒤집혀 있다");
			_bounds = bounds;
		}

		Bounds WorldBounds() const { return _bounds; }

		// 보이는 엔티티를 칸 순서로 (그리는 순서·충돌 쌍 순서가 안정적)
		// auto 반환 → 클래스 안에서 쓰는 곳보다 먼저 정의 (GCC)
		auto Entities()
		{
			return std::views::iota(std::uint32_t{ 0 }, _entities.SlotCount())
				| std::views::filter([this](std::uint32_t index) { return _entities.IsVisible(index); })
				| std::views::transform([this](std::uint32_t index) { return Entity{ _entities.IdAt(index), this }; });
		}

		// First 저장소를 빈틈없이 돌며, 나머지 컴포넌트도 가진 것만 body(Entity, First&, Rest&...)
		// 순서는 저장소 순서라 삭제 후 바뀔 수 있음. body 안에서 Add·Spawn 금지(재배치)
		template<class First, class... Rest, class F>
		void Each(F&& body)
		{
			SparseSet<First>& first = _components.Of<First>();

			++_iterating;
			for (std::size_t position = 0; position < first.Size(); ++position)
			{
				std::uint32_t index = first.KeyAt(position);
				if (!_entities.IsVisible(index) || !(_components.Of<Rest>().Contains(index) && ...))
					continue;

				body(Entity{ _entities.IdAt(index), this }, first.At(position), _components.Of<Rest>().Get(index)...);
			}
			--_iterating;
		}

		// 나중에 그린 것이 위 → 역순 탐색
		Entity HitTest(Vec2 point)
		{
			for (Entity entity : Entities() | std::views::reverse)
				if (entity.GetBounds().Contains(point))
					return entity;
			return {};
		}

		Entity FindOverlap(Entity target)
		{
			for (Entity entity : Entities() | std::views::reverse)
				if (entity != target && entity.GetBounds().Intersects(target.GetBounds()))
					return entity;
			return {};
		}

		std::size_t Size() const { return _entities.VisibleCount(); }
		bool IsEmpty() const { return Size() == 0; }

	private:
		friend class Entity;

		EntityTable _entities;
		ComponentPools _components;
		int _iterating{};						// Each 중첩 깊이. 0이 아니면 Add·Spawn 금지
		Bounds _bounds{ .min{ -1.f, -1.f }, .max{ 1.f, 1.f } };	// 기본값: NDC 전체
	};

	// Entity 멤버는 World가 완전해진 뒤에 정의
	inline Entity::operator bool() const
	{
		return _world && _world->_entities.IsValid(_id);
	}

	inline bool Entity::IsAlive() const
	{
		return _world && _world->_entities.IsAlive(_id);
	}

	template<class T, class... Args>
	T& Entity::Add(Args&&... args) const
	{
		Check(static_cast<bool>(*this), "없는 엔티티에 컴포넌트 추가");
		Check(_world->_iterating == 0, "Each 도중 Add 금지: 저장소가 재배치되어 순회 중인 참조가 깨진다");
		Check(!Get<T>(), "이미 붙어 있다. 값을 바꾸려면 Get을 쓸 것");

		return _world->_components.Of<T>().Add(_id.index, std::forward<Args>(args)...);
	}

	template<class T>
	T* Entity::Get() const
	{
		if (!*this)
			return nullptr;

		SparseSet<T>& set = _world->_components.Of<T>();
		return set.Contains(_id.index) ? &set.Get(_id.index) : nullptr;
	}

	template<class T>
	bool Entity::Remove() const
	{
		Check(!_world || _world->_iterating == 0, "Each 도중 Remove 금지");
		if (!Get<T>())
			return false;

		_world->_components.Of<T>().Remove(_id.index);
		return true;
	}
}
