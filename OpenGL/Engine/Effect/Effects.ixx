export module hs.effects;

import std;
import hs.color;
import hs.easing;
export import hs.effect;

export namespace hs
{
	class ScalePulse final : public IEffect
	{
	public:
		ScalePulse(float min = 0.6f, float max = 1.4f, float period = 2.f)
			: _min(min), _max(max), _playback(period) {}

		void Update(Visual& visual, float dt) override
		{
			float t = EaseInOut(PingPong(_playback.Advance(dt)));
			visual.effectScale = std::lerp(_min, _max, t);
		}

		void Restore(Visual& visual) override { visual.effectScale = 1.f; }

	private:
		float _min;
		float _max;
		Playback _playback;
	};

	// start: 개체별 시작 색 분산용
	class ColorCycle final : public IEffect
	{
	public:
		explicit ColorCycle(float period = 3.f, float start = 0.f)
			: _playback(period)
		{
			_playback.Seek(start);
		}

		void Update(Visual& visual, float dt) override
		{
			visual.color = FromHsv(_playback.Advance(dt));
		}

	private:
		Playback _playback;
	};

	class ScaleIn final : public IEffect
	{
	public:
		explicit ScaleIn(float from = 0.f, float duration = 0.15f)
			: _from(from), _playback(duration, false) {}

		void Update(Visual& visual, float dt) override
		{
			float t = EaseOutBack(_playback.Advance(dt));
			visual.effectScale = std::lerp(_from, 1.f, t);
		}

		bool IsFinished() const override { return _playback.IsFinished(); }

		void Restore(Visual& visual) override { visual.effectScale = 1.f; }

	private:
		float _from;
		Playback _playback;
	};
}
