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

	// 검정으로 가면 점점 어둡게, 흰색으로 가면 점점 밝게. 끝나면 to에 머묾
	class ColorFade final : public IEffect
	{
	public:
		ColorFade(Color from, Color to, float duration)
			: _from(from), _to(to), _playback(duration, false) {}

		void Update(Visual& visual, float dt) override
		{
			visual.color = Lerp(_from, _to, _playback.Advance(dt));
		}

		bool IsFinished() const override { return _playback.IsFinished(); }

		// EffectSystem이 끝난 효과를 뺄 때도 부름 → _from이면 끝나는 순간 원래 색으로 튐
		void Restore(Visual& visual) override { visual.color = _to; }

	private:
		Color _from;
		Color _to;
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
