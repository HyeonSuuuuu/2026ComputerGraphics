export module hs.animation;

import std;
import hs.check;
export import hs.component;
export import hs.scene;
export import hs.type_id;

// 충돌이 안 건드리는 값만 (크기·색·속력)
export namespace hs
{
	// 왕복 여부는 이징 담당
	class Playback
	{
	public:
		explicit Playback(float duration = 1.f, bool loop = true)
			: _duration(duration), _loop(loop)
		{
			Check(duration > 0.f, "길이가 0이면 값이 매 프레임 튄다");
		}

		float Advance(float dt)
		{
			_time += dt;
			_time = _loop ? std::fmod(_time, _duration) : std::min(_time, _duration);
			return _time / _duration;
		}

		void Seek(float phase)
		{
			Check(phase >= 0.f && phase <= 1.f, "위상은 0~1이다");
			_time = phase * _duration;
		}

		bool IsFinished() const { return !_loop && _time >= _duration; }

	private:
		float _duration;
		bool _loop;
		float _time{};
	};

	// 위치 제외: 속도로 표현해야 경계 처리 적용
	class IAnimation
	{
	public:
		virtual ~IAnimation() = default;

		virtual void Update(Object& object, float dt) = 0;

		// 끄기와 되돌리기를 한 곳에. 따로 두면 한쪽 누락
		void SetEnabled(Object& object, bool on)
		{
			_enabled = on;
			if (!on)
				Restore(object);
		}

		bool IsEnabled() const { return _enabled; }

		// true면 목록에서 제거
		virtual bool IsFinished() const { return false; }

		virtual void Restore(Object&) {}

	private:
		bool _enabled{ true };
	};

	// 서로 다른 값 담당 → 동시 실행 가능
	struct Animator : IComponent
	{
		template<class T, class... Args>
		T& Add(Args&&... args)
		{
			static_assert(std::derived_from<T, IAnimation>, "애니메이션은 IAnimation을 상속해야 한다");
			Check(!Get<T>(), "이미 붙어 있다. 값을 바꾸려면 Get을 쓸 것");

			auto created = std::make_unique<T>(std::forward<Args>(args)...);
			T& added = *created;
			animations.emplace_back(TypeIdOf<T>, std::move(created));
			return added;
		}

		template<class T>
		T* Get()
		{
			for (auto& [type, animation] : animations)
				if (type == TypeIdOf<T>)
					return static_cast<T*>(animation.get());
			return nullptr;
		}

		std::vector<std::pair<TypeId, std::unique_ptr<IAnimation>>> animations;
		bool enabled{ true };
	};
}
