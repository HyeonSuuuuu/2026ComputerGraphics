export module hs.animations;

import std;
import hs.check;
export import hs.vec2;
import hs.color;
import hs.easing;
import hs.scene;
export import hs.animation;
import hs.visual;

export namespace hs
{
	// 보이는 크기만. transform.size 불변
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

			float t = EaseInOut(PingPong(_playback.Advance(dt)));
			visual->effectScale = std::lerp(_min, _max, t);
		}

		void Restore(Object& object) override
		{
			if (Visual* visual = object.Get<Visual>())
				visual->effectScale = 1.f;
		}

	private:
		float _min;
		float _max;
		Playback _playback;
	};

	// start: 개체별 시작 색 분산용
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
	
	class ScaleIn final : public IAnimation
	{
	public:
		explicit ScaleIn(float from = 0.f, float duration = 0.15f)
			: _from(from), _playback(duration, false) {}

		void Update(Object& object, float dt) override
		{
			Visual* visual = object.Get<Visual>();
			if (!visual)
				return;

			float t = EaseOutBack(_playback.Advance(dt)); 
			visual->effectScale = std::lerp(_from, 1.f, t);
		}

		bool IsFinished() const override { return _playback.IsFinished(); }

		void Restore(Object& object) override
		{
			if (Visual* visual = object.Get<Visual>())
				visual->effectScale = 1.f;
		}

	private:
		float _from;
		Playback _playback;
	};

	// 도중 목표 변경 허용
	// 끝이 없어 Playback 불필요
	class SizeChase final : public IAnimation
	{
	public:
		explicit SizeChase(Vec2 target, float rate = 10.f)
			: _target(target), _rate(rate)
		{
			Check(rate > 0.f, "따라가는 속도가 0이면 영영 도착하지 않는다");
		}

		void SetTarget(Vec2 target) { _target = target; }
		Vec2 Target() const { return _target; }

		void Update(Object& object, float dt) override
		{
			// 큰 dt에서도 목표 초과 방지
			object.transform.size += (_target - object.transform.size) * std::min(_rate * dt, 1.f);
		}

	private:
		Vec2 _target;
		float _rate;
	};
}
