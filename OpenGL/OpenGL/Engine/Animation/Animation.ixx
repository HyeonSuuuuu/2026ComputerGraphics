module;
#include <typeinfo>

export module hs.animation;

import std;
import hs.check;
import hs.component;
import hs.object;

// 시간 함수로 값을 정한다. 충돌이 건드리지 않는 값만 (크기·색·속력)
export namespace hs
{
	// 애니메이션의 시간 축. 0~1을 돌려주고, 왕복 여부는 이징이 정한다
	struct Playback
	{
		explicit Playback(float duration = 1.f, bool loop = true)
			: duration(duration), loop(loop)
		{
			Check(duration > 0.f, "길이가 0이면 값이 매 프레임 튄다");
		}

		float Advance(float dt)
		{
			time += dt;
			time = loop ? std::fmod(time, duration) : std::min(time, duration);
			return time / duration;
		}

		bool IsFinished() const { return !loop && time >= duration; }

		float duration;
		bool loop;
		float time{};
	};

	// 시간에 따라 오브젝트의 값을 바꾼다
	// 위치는 여기서 다루지 않는다. 속도로 표현해야 경계 처리가 적용된다 (ILayeredMove)
	class IAnimation
	{
	public:
		virtual ~IAnimation() = default;

		virtual void Update(Object& object, float dt) = 0;

		// 한 번만 재생되는 애니메이션은 끝나면 목록에서 빠진다
		virtual bool IsFinished() const { return false; }

		// 꺼질 때 건드린 값을 원래대로. 되돌릴 게 없으면 그냥 둔다
		virtual void Restore(Object& object) {}

		bool enabled{ true };
	};

	// 오브젝트에 얹힌 애니메이션들. 서로 다른 값을 건드리므로 여러 개가 같이 돈다
	struct Animator : IComponent
	{
		template<class T, class... Args>
		T& Add(Args&&... args)
		{
			static_assert(std::derived_from<T, IAnimation>, "애니메이션은 IAnimation을 상속해야 한다");
			Check(!Get<T>(), "이미 붙어 있다. 값을 바꾸려면 Get을 쓸 것");

			auto created = std::make_unique<T>(std::forward<Args>(args)...);
			T& added = *created;
			animations.emplace_back(typeid(T), std::move(created));
			return added;
		}

		template<class T>
		T* Get()
		{
			for (auto& [type, animation] : animations)
				if (type == typeid(T))
					return static_cast<T*>(animation.get());
			return nullptr;
		}

		std::vector<std::pair<std::type_index, std::unique_ptr<IAnimation>>> animations;
		bool enabled{ true };
	};
}
