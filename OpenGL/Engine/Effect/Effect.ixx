module;
#include "Core/Check.h"

export module hs.effect;

import std;
import hs.check;
export import hs.visual;
export import hs.type_id;

// 보이는 것(Visual)만 바꿈 → 꺼도 게임 결과는 같아야 함. 게임 값 변화는 게임 로직·시스템 몫
export namespace hs
{
	// 왕복 여부는 이징 담당
	class Playback
	{
	public:
		explicit Playback(float duration = 1.f, bool loop = true)
			: _duration(duration), _loop(loop)
		{
			HS_DCHECK(duration > 0.f, "길이가 0이면 값이 매 프레임 튄다");
		}

		float Advance(float dt)
		{
			_time += dt;
			_time = _loop ? std::fmod(_time, _duration) : std::min(_time, _duration);
			return _time / _duration;
		}

		void Seek(float phase)
		{
			HS_DCHECK(phase >= 0.f && phase <= 1.f, "위상은 0~1이다");
			_time = phase * _duration;
		}

		bool IsFinished() const { return !_loop && _time >= _duration; }

	private:
		float _duration;
		bool _loop;
		float _time{};
	};

	class IEffect
	{
	public:
		virtual ~IEffect() = default;

		virtual void Update(Visual& visual, float dt) = 0;

		// 끄기와 되돌리기를 한 곳에. 따로 두면 한쪽 누락
		void SetEnabled(Visual& visual, bool on)
		{
			_enabled = on;
			if (!on)
				Restore(visual);
		}

		bool IsEnabled() const { return _enabled; }

		// true면 목록에서 제거
		virtual bool IsFinished() const { return false; }

		virtual void Restore(Visual&) {}

	private:
		bool _enabled{ true };
	};

	// 서로 다른 값 담당 → 동시 실행 가능
	struct EffectStack
	{
		template<class T, class... Args>
		T& Add(Args&&... args)
		{
			static_assert(std::derived_from<T, IEffect>, "이펙트는 IEffect를 상속해야 한다");
			HS_DCHECK(!Get<T>(), "이미 붙어 있다. 값을 바꾸려면 Get을 쓸 것");

			auto created = std::make_unique<T>(std::forward<Args>(args)...);
			T& added = *created;
			effects.emplace_back(TypeIdOf<T>, std::move(created));
			return added;
		}

		template<class T>
		T* Get()
		{
			for (auto& [type, effect] : effects)
				if (type == TypeIdOf<T>)
					return static_cast<T*>(effect.get());
			return nullptr;
		}

		std::vector<std::pair<TypeId, std::unique_ptr<IEffect>>> effects;
		bool enabled{ true };
	};
}
