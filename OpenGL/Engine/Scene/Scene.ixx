export module hs.scene;

import std;
import hs.check;
import hs.type_id;
import hs.sparse_set;
export import hs.transform;

export namespace hs
{
	class Scene;

	// generation 0 = 빈 값. 소멸 후 같은 칸이 재사용돼도 세대가 달라 옛 ID로는 안 잡힘
	struct ObjectId
	{
		std::uint32_t index{};
		std::uint32_t generation{};

		bool operator==(const ObjectId&) const = default;
		explicit operator bool() const { return generation != 0; }
	};

	// 컴포넌트를 들고 있지 않은 손잡이(번호 + Scene). 값으로 복사해 넘김
	// Get으로 받은 포인터는 그 자리에서만: 같은 타입의 Add·삭제가 저장소를 재배치함
	class Object
	{
	public:
		Object() = default;

		// 존재하는 오브젝트를 가리키는지 (Flush 전·Destroy 후도 true. 목록 노출 여부는 Scene::Find)
		explicit operator bool() const;
		bool operator==(const Object&) const = default;

		ObjectId Id() const { return _id; }
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
		friend class Scene;
		Object(ObjectId id, Scene* scene) : _id(id), _scene(scene) {}

		ObjectId _id;
		Scene* _scene{};
	};

	// 컴포넌트를 타입별 SparseSet에 보관. 목록 변경(생성 노출·소멸)은 Flush에서만
	class Scene
	{
	public:
		Scene() = default;
		Scene(const Scene&) = delete;				// Object가 Scene 주소를 들고 있음
		Scene& operator=(const Scene&) = delete;

		Object Spawn(const Transform& transform)
		{
			Check(transform.size.x > 0.f && transform.size.y > 0.f, "크기가 0인 오브젝트는 보이지도 맞지도 않는다");

			std::uint32_t index = Acquire();
			Slot& slot = _slots[index];
			slot.used = true;
			slot.alive = true;
			_pending.push_back(index);

			Object object{ { index, slot.generation }, this };
			object.Add<Transform>(transform);
			return object;
		}

		void Destroy(Object object)
		{
			if (IsValid(object._id))
				_slots[object._id.index].alive = false;
		}

		// 목록에 보이는 것만. 아니면 빈 Object
		Object Find(ObjectId id)
		{
			return IsValid(id) && IsVisible(id.index) ? Object{ id, this } : Object{};
		}

		// 순회 중 호출 금지
		void Flush()
		{
			Check(_iterating == 0, "Each 도중 Flush 금지");

			for (std::uint32_t index : _pending)
			{
				Slot& slot = _slots[index];
				if (slot.used && slot.alive)
				{
					slot.active = true;
					++_count;
				}
			}
			_pending.clear();

			for (std::uint32_t index = 0; index < _slots.size(); ++index)
			{
				Slot& slot = _slots[index];
				if (!slot.used || slot.alive)
					continue;

				if (slot.active)
					--_count;

				RemoveComponents(index);
				slot = { .generation = slot.generation + 1 };	// 이전 ObjectId 무효화 지점
				_free.push_back(index);
			}
		}

		void Clear()
		{
			Check(_iterating == 0, "Each 도중 Clear 금지");

			for (auto& pool : _pools)
				if (pool)
					pool->Clear();

			_free.clear();
			for (std::uint32_t index = 0; index < _slots.size(); ++index)
			{
				Slot& slot = _slots[index];
				if (slot.used)
					slot = { .generation = slot.generation + 1 };
				_free.push_back(index);
			}
			_pending.clear();
			_count = 0;
		}

		// 레벨 변경 시에만 갱신
		void SetBounds(Bounds bounds)
		{
			Check(bounds.min.x <= bounds.max.x && bounds.min.y <= bounds.max.y, "bounds의 min과 max가 뒤집혀 있다");
			_bounds = bounds;
		}

		Bounds WorldBounds() const { return _bounds; }

		// 보이는 오브젝트를 칸 순서로 (그리는 순서·충돌 쌍 순서가 안정적)
		// auto 반환 → 클래스 안에서 쓰는 곳보다 먼저 정의 (GCC)
		auto Objects()
		{
			return std::views::iota(std::uint32_t{ 0 }, static_cast<std::uint32_t>(_slots.size()))
				| std::views::filter([this](std::uint32_t index) { return IsVisible(index); })
				| std::views::transform([this](std::uint32_t index) { return Object{ { index, _slots[index].generation }, this }; });
		}

		// First 저장소를 빈틈없이 돌며, 나머지 컴포넌트도 가진 것만 body(Object, First&, Rest&...)
		// 순서는 저장소 순서라 삭제 후 바뀔 수 있음. body 안에서 Add·Spawn 금지(재배치)
		template<class First, class... Rest, class F>
		void Each(F&& body)
		{
			SparseSet<First>& first = PoolOf<First>().set;

			++_iterating;
			for (std::size_t position = 0; position < first.Size(); ++position)
			{
				std::uint32_t index = first.KeyAt(position);
				if (!IsVisible(index) || !(PoolOf<Rest>().set.Contains(index) && ...))
					continue;

				body(Object{ { index, _slots[index].generation }, this }, first.At(position), PoolOf<Rest>().set.Get(index)...);
			}
			--_iterating;
		}

		// 나중에 그린 것이 위 → 역순 탐색
		Object HitTest(Vec2 point)
		{
			for (Object object : Objects() | std::views::reverse)
				if (object.GetBounds().Contains(point))
					return object;
			return {};
		}

		Object FindOverlap(Object target)
		{
			for (Object object : Objects() | std::views::reverse)
				if (object != target && object.GetBounds().Intersects(target.GetBounds()))
					return object;
			return {};
		}

		std::size_t Size() const { return _count; }
		bool IsEmpty() const { return _count == 0; }

	private:
		friend class Object;

		struct Slot
		{
			std::uint32_t generation{ 1 };	// 0 = 무효 ObjectId → 1부터
			bool used{};					// 이 칸에 오브젝트가 있음
			bool alive{};					// Destroy 전
			bool active{};					// Flush를 거쳐 목록에 보임
		};

		// 타입을 모르는 채로 삭제하기 위한 끼우는 자리
		struct IPool
		{
			virtual ~IPool() = default;
			virtual void Remove(std::uint32_t index) = 0;
			virtual void Clear() = 0;
		};

		template<class T>
		struct Pool final : IPool
		{
			SparseSet<T> set;

			void Remove(std::uint32_t index) override
			{
				if (set.Contains(index))
					set.Remove(index);
			}

			void Clear() override { set.Clear(); }
		};

		template<class T>
		Pool<T>& PoolOf()
		{
			std::uint32_t type = TypeIndexOf<T>();
			if (type >= _pools.size())
				_pools.resize(type + 1);
			if (!_pools[type])
				_pools[type] = std::make_unique<Pool<T>>();
			return static_cast<Pool<T>&>(*_pools[type]);
		}

		bool IsValid(ObjectId id) const
		{
			return id && id.index < _slots.size() && _slots[id.index].used && _slots[id.index].generation == id.generation;
		}

		bool IsVisible(std::uint32_t index) const
		{
			const Slot& slot = _slots[index];
			return slot.used && slot.alive && slot.active;
		}

		void RemoveComponents(std::uint32_t index)
		{
			for (auto& pool : _pools)
				if (pool)
					pool->Remove(index);
		}

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
		std::vector<std::unique_ptr<IPool>> _pools;	// TypeIndexOf<T>가 칸 번호
		std::vector<std::uint32_t> _free;
		std::vector<std::uint32_t> _pending;		// Flush에서 노출할 칸
		std::size_t _count{};						// 보이는 오브젝트 수
		int _iterating{};							// Each 중첩 깊이. 0이 아니면 Add·Spawn 금지
		Bounds _bounds{ .min{ -1.f, -1.f }, .max{ 1.f, 1.f } };	// 기본값: NDC 전체
	};

	// Object 멤버는 Scene이 완전해진 뒤에 정의
	inline Object::operator bool() const
	{
		return _scene && _scene->IsValid(_id);
	}

	inline bool Object::IsAlive() const
	{
		return *this && _scene->_slots[_id.index].alive;
	}

	template<class T, class... Args>
	T& Object::Add(Args&&... args) const
	{
		Check(static_cast<bool>(*this), "없는 오브젝트에 컴포넌트 추가");
		Check(_scene->_iterating == 0, "Each 도중 Add 금지: 저장소가 재배치되어 순회 중인 참조가 깨진다");
		Check(!Get<T>(), "이미 붙어 있다. 값을 바꾸려면 Get을 쓸 것");

		return _scene->PoolOf<T>().set.Add(_id.index, std::forward<Args>(args)...);
	}

	template<class T>
	T* Object::Get() const
	{
		if (!*this)
			return nullptr;

		SparseSet<T>& set = _scene->PoolOf<T>().set;
		return set.Contains(_id.index) ? &set.Get(_id.index) : nullptr;
	}

	template<class T>
	bool Object::Remove() const
	{
		Check(_scene == nullptr || _scene->_iterating == 0, "Each 도중 Remove 금지");
		if (!Get<T>())
			return false;

		_scene->PoolOf<T>().set.Remove(_id.index);
		return true;
	}
}
