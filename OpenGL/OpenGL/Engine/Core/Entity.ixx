export module hs.entity;

import std;
import hs.check;

export namespace hs
{
	// 오브젝트 하나를 가리키는 ID. 저장소들이 이 ID로 같은 오브젝트를 공유한다
	using EntityId = std::uint32_t;

	inline constexpr EntityId NullEntity = std::numeric_limits<EntityId>::max();
	
	class EntityIdPool
	{
	public:
		
		EntityId Create()
		{
			Check(_next != NullEntity, "EntityId를 모두 소진했다");
			return _next++;
		}
		
		void Reserve(EntityId id)
		{
			if (id != NullEntity && id >= _next)
				_next = id + 1;
		}

		void Reset() { _next = 0; }

	private:
		EntityId _next{};
	};
}
