export module hs.component_pools;

import std;
import hs.type_id;
export import hs.sparse_set;

export namespace hs
{
	// 컴포넌트 타입마다 SparseSet 하나. 키는 엔티티 칸 번호
	// 저장 방식을 바꾸게 되면(EnTT, group 등) 이 파일만 교체
	class ComponentPools
	{
	public:
		template<class T>
		SparseSet<T>& Of()
		{
			std::uint32_t type = TypeIndexOf<T>();
			if (type >= _pools.size())
				_pools.resize(type + 1);
			if (!_pools[type])
				_pools[type] = std::make_unique<Pool<T>>();
			return static_cast<Pool<T>&>(*_pools[type]).set;
		}

		// 엔티티 소멸 시: 어떤 타입을 가졌는지 몰라도 전부에서 제거
		void RemoveAll(std::uint32_t index)
		{
			for (auto& pool : _pools)
				if (pool)
					pool->Remove(index);
		}

		void Clear()
		{
			for (auto& pool : _pools)
				if (pool)
					pool->Clear();
		}

	private:
		// 타입을 모르는 채로 지우기 위한 끼우는 자리
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

		std::vector<std::unique_ptr<IPool>> _pools;	// TypeIndexOf<T>가 칸 번호
	};
}
