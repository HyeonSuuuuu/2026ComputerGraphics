module;
#include <gl/glm/glm.hpp>

export module hs.animations;

import std;
import hs.check;
import hs.shape;
import hs.easing;
import hs.object;
import hs.animation;
import hs.visual;

export namespace hs
{
	// 보이는 크기가 커졌다 작아졌다를 반복한다. 논리 크기(transform.size)는 건드리지 않는다
	class ScalePulse final : public IAnimation
	{
	public:
		ScalePulse(float min = 0.6f, float max = 1.4f, float period = 2.f)
			: _min(min), _max(max), _playback(period) {}

		void Update(Object& object, float dt) override
		{
			Visual* visual = object.Get<Visual>();
			if (!visual)
				return;

			// PingPong이 0→1→0을 만들고, EaseInOut이 양 끝을 부드럽게 한다
			float t = EaseInOut(PingPong(_playback.Advance(dt)));
			visual->scale = std::lerp(_min, _max, t);
		}

		void Restore(Object& object) override
		{
			if (Visual* visual = object.Get<Visual>())
				visual->scale = 1.f;
		}

	private:
		float _min;
		float _max;
		Playback _playback;
	};

	// 색상환을 한 바퀴 돈다. start로 각자 다른 색에서 출발시킨다
	class ColorCycle final : public IAnimation
	{
	public:
		explicit ColorCycle(float period = 3.f, float start = 0.f)
			: _playback(period)
		{
			_playback.Seek(start);
		}

		void Update(Object& object, float dt) override
		{
			Visual* visual = object.Get<Visual>();
			if (!visual)
				return;

			visual->color = FromHsv(_playback.Advance(dt));
		}

	private:
		Playback _playback;
	};
}
