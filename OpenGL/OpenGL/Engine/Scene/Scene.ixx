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
		Object& Spawn(const Transform& transform)
		{
			Check(transform.size.x > 0.f && transform.size.y > 0.f, "크기가 0인 오브젝트는 보이지도 맞지도 않는다");
			
			_objects.push_back(std::make_unique<Object>(transform));
			return *_objects.back();
		}

		bool Destroy(const Object* object)
		{
			auto it = std::ranges::find_if(_objects, [object](const auto& o) { return o.get() == object; });
			if (it == _objects.end())
				return false;

			_objects.erase(it);
			return true;
		}

		void Clear() { _objects.clear(); }

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
				if (object->GetBounds().Contains(point))
					return object.get();
			return nullptr;
		}

		Object* FindOverlap(const Object& target)
		{
			for (auto& object : _objects | std::views::reverse)
				if (object.get() != &target && object->GetBounds().Intersects(target.GetBounds()))
					return object.get();
			return nullptr;
		}

		auto Objects() { return _objects | std::views::transform([](auto& o) -> Object& { return *o; }); }
		auto Objects() const { return _objects | std::views::transform([](const auto& o) -> const Object& { return *o; }); }

		std::size_t Size() const { return _objects.size(); }
		bool IsEmpty() const { return _objects.empty(); }

	private:
		std::vector<std::unique_ptr<Object>> _objects;
		Bounds _bounds{ .min{ -1.f, -1.f }, .max{ 1.f, 1.f } };	// 기본값은 NDC 전체
	};
}
