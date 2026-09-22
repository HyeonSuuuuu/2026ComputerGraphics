export module hs.scene;

import std;
import hs.check;
import hs.shape;

export import hs.object; // 오브젝트에 의존함, 쓰는 쪽이 편하게 함께 내보냄

export namespace hs
{
	class Scene
	{
	public:
		// 목록은 Flush에서만 바뀐다. 순회 중 어디서 불러도 안전하다
		Object& Spawn(const Transform& transform)
		{
			Check(transform.size.x > 0.f && transform.size.y > 0.f, "크기가 0인 오브젝트는 보이지도 맞지도 않는다");

			_spawned.push_back(std::make_unique<Object>(transform));
			return *_spawned.back();
		}

		void Destroy(Object& object) { object._alive = false; }

		// 프레임의 정해진 한 지점에서 부른다. 순회 중에 부르면 안 된다
		void Flush()
		{
			_objects.insert(_objects.end(),
				std::make_move_iterator(_spawned.begin()), std::make_move_iterator(_spawned.end()));
			_spawned.clear();

			std::erase_if(_objects, [](const auto& object) { return !object->IsAlive(); });
		}

		void Clear()
		{
			_objects.clear();
			_spawned.clear();
		}

		// 오브젝트가 돌아다닐 수 있는 영역. 레벨이 바뀔 때만 갱신하면 된다
		void SetBounds(Bounds bounds)
		{
			Check(bounds.min.x <= bounds.max.x && bounds.min.y <= bounds.max.y, "bounds의 min과 max가 뒤집혀 있다");
			_bounds = bounds;
		}

		Bounds GetBounds() const { return _bounds; }

		// 나중에 그려진 것이 위에 있으므로 뒤에서부터 찾는다
		Object* HitTest(Vec2 point)
		{
			for (auto& object : _objects | std::views::reverse)
				if (object->IsAlive() && object->GetBounds().Contains(point))
					return object.get();
			return nullptr;
		}

		Object* FindOverlap(const Object& target)
		{
			for (auto& object : _objects | std::views::reverse)
				if (object.get() != &target && object->IsAlive() && object->GetBounds().Intersects(target.GetBounds()))
					return object.get();
			return nullptr;
		}

		auto Objects() { return _objects | std::views::transform([](auto& o) -> Object& { return *o; }); }
		auto Objects() const { return _objects | std::views::transform([](const auto& o) -> const Object& { return *o; }); }

		std::size_t Size() const { return _objects.size(); }
		bool IsEmpty() const { return _objects.empty(); }

	private:
		std::vector<std::unique_ptr<Object>> _objects;
		std::vector<std::unique_ptr<Object>> _spawned;	// Flush 때 _objects로 들어간다
		Bounds _bounds{ .min{ -1.f, -1.f }, .max{ 1.f, 1.f } };	// 기본값은 NDC 전체
	};
}
