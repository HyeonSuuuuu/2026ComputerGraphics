module;
#include <typeinfo>

export module hs.object;

import std;
import hs.check;
import hs.shape;
import hs.component;

export namespace hs
{
	// 오브젝트를 프레임 넘어 가리키는 손잡이. 포인터와 달리 죽은 뒤에도 안전하게 물어볼 수 있다
	// generation이 0이면 아무것도 가리키지 않는다
	struct ObjectId
	{
		std::uint32_t index{};
		std::uint32_t generation{};

		bool operator==(const ObjectId&) const = default;
		explicit operator bool() const { return generation != 0; }
	};

	// 화면에 존재하는 것 하나. 데이터를 들고 있고, 기능은 컴포넌트로 붙인다
	// 동작 자체는 컴포넌트와 시스템이 맡는다. 여기에 로직을 넣지 말 것
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

		// Destroy된 오브젝트는 Flush 전까지 목록에 남아 있다
		bool IsAlive() const { return _alive; }
		ObjectId Id() const { return _id; }

	private:
		friend class Scene;
		bool _alive = true;
		ObjectId _id;

		// 보통 두세 개라 선형 탐색이 해시보다 빠르다
		std::vector<std::pair<std::type_index, std::unique_ptr<IComponent>>> _components;
	};
}
