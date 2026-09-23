module;
#include <typeinfo>

// Scene을 friend로 두려면 같은 모듈이어야 함 → hs.scene의 파티션
export module hs.scene:object;

import std;
import hs.check;
import hs.shape;
import hs.component;

export namespace hs
{
	// 포인터와 달리 소멸 후에도 안전한 조회
	// generation 0 = 빈 값
	struct ObjectId
	{
		std::uint32_t index{};
		std::uint32_t generation{};

		bool operator==(const ObjectId&) const = default;
		explicit operator bool() const { return generation != 0; }
	};

	// 로직 금지. 동작은 컴포넌트와 시스템 담당
	class Object
	{
	public:
		explicit Object(const Transform& transform) : transform(transform) {}

		Transform transform;

		template<class T, class... Args>
		T& Add(Args&&... args)
		{
			static_assert(std::derived_from<T, IComponent>, "컴포넌트는 IComponent를 상속해야 한다");
			Check(!Get<T>(), "이미 붙어 있다. 값을 바꾸려면 Get을 쓸 것");

			auto created = std::make_unique<T>(std::forward<Args>(args)...);
			T& added = *created;
			_components.emplace_back(typeid(T), std::move(created));
			return added;
		}

		template<class T>
		T* Get()
		{
			for (auto& [type, component] : _components)
				if (type == typeid(T))
					return static_cast<T*>(component.get());
			return nullptr;
		}

		template<class T>
		const T* Get() const
		{
			for (const auto& [type, component] : _components)
				if (type == typeid(T))
					return static_cast<const T*>(component.get());
			return nullptr;
		}

		template<class T>
		bool Remove()
		{
			auto it = std::ranges::find_if(_components, [](const auto& e) { return e.first == typeid(T); });
			if (it == _components.end())
				return false;

			_components.erase(it);
			return true;
		}

		Bounds GetBounds() const { return transform.GetBounds(); }

		// Destroy 후에도 Flush 전까지 목록에 잔존
		bool IsAlive() const { return _alive; }
		ObjectId Id() const { return _id; }

	private:
		friend class Scene;
		bool _alive = true;
		ObjectId _id;

		// 보통 두세 개 → 선형 탐색이 해시보다 빠름
		std::vector<std::pair<std::type_index, std::unique_ptr<IComponent>>> _components;
	};
}
