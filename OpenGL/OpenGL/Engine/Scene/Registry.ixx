export module hs.registry;

import std;
import hs.check;
import hs.shape;
import hs.entity;
import hs.rect_layer;
import hs.movement_system;

export namespace hs
{
	// 엔티티의 수명과 저장소들을 소유한다. 게임 개념(맵, 카메라, 시스템 순서)은 모른다
	// 저장소끼리는 서로를 모르고, 여기서만 같은 EntityId로 묶인다
	class Registry
	{
	public:
		EntityId Spawn(const Rectangle& rect)
		{
			Check(rect.size.x > 0.f && rect.size.y > 0.f, "크기가 0인 오브젝트는 보이지도 맞지도 않는다");

			EntityId id = _ids.Create();
			_rects.Add(id, rect);
			return id;
		}

		// 모든 저장소에서 제거한다. 저장소가 늘어나면 여기에 한 줄씩 추가
		void Destroy(EntityId id)
		{
			_rects.Remove(id);
			_movement.Detach(id);
		}

		void Clear()
		{
			_rects.Clear();
			_movement.Clear();
			_ids.Reset();
		}

		RectLayer& Rects() { return _rects; }
		const RectLayer& Rects() const { return _rects; }

		MovementSystem& Movement() { return _movement; }
		const MovementSystem& Movement() const { return _movement; }

	private:
		EntityIdPool	_ids;
		RectLayer		_rects;
		MovementSystem	_movement;
	};
}
