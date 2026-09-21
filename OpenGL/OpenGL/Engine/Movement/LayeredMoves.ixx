module;
#include <gl/glm/glm.hpp>

export module hs.layered_moves;

import std;
import hs.check;
import hs.shape;
import hs.easing;
import hs.movement;

export namespace hs
{
	// 정해진 시간 동안 목표 위치로 이동한다 (Tween을 속도로 바꿔 넘기는 방식)
	// 위치를 직접 쓰지 않으므로 경계 처리가 그대로 적용된다
	class MoveTo final : public ILayeredMove
	{
	public:
		MoveTo(Vec2 target, float duration, EaseFn ease = Linear)
			: _target(target), _duration(duration), _ease(std::move(ease))
		{
			Check(duration > 0.f, "0초 이동은 의미가 없다. 위치를 직접 옮겨라");
			Check(_ease != nullptr, "이징 함수가 비어 있다");
		}

		MixMode GetMixMode() const override { return MixMode::Override; }
		bool IsFinished() const override { return _elapsed >= _duration; }

		Vec2 Evaluate(const Transform& transform, float dt) override
		{
			if (!_started)			// 시작 위치는 생성 시점이 아니라 첫 평가 때 잡는다
			{
				_start = transform.pos;
				_started = true;
			}

			if (dt <= 0.f || _duration <= 0.f)
				return {};

			_elapsed = std::min(_elapsed + dt, _duration);

			// 이번 프레임에 있어야 할 위치까지 가는 데 필요한 속도
			Vec2 desired = Lerp(_start, _target, _ease(_elapsed / _duration));
			return (desired - transform.pos) / dt;
		}

	private:
		Vec2 _start{};
		Vec2 _target;
		float _duration;
		float _elapsed{};
		EaseFn _ease;
		bool _started{};
	};

	// 넉백, 대시처럼 잠깐 밀어내는 힘. 기존 속도 위에 더해진다
	// 이동 거리는 speed * duration * (커브 아래 넓이)
	class Impulse final : public ILayeredMove
	{
	public:
		Impulse(Vec2 dir, float speed, float duration, EaseFn curve = Reverse)
			: _dir(Normalize(dir)), _speed(speed), _duration(duration), _curve(std::move(curve))
		{
			Check(LengthSq(dir) > 0.f, "방향이 0이면 아무 일도 일어나지 않는다");
			Check(speed >= 0.f);
			Check(duration > 0.f);
			Check(_curve != nullptr, "커브 함수가 비어 있다");
		}

		MixMode GetMixMode() const override { return MixMode::Additive; }
		bool IsFinished() const override { return _elapsed >= _duration; }

		Vec2 Evaluate(const Transform&, float dt) override
		{
			if (_duration <= 0.f)
				return {};

			_elapsed = std::min(_elapsed + dt, _duration);
			return _dir * (_speed * _curve(_elapsed / _duration));
		}

	private:
		Vec2 _dir;
		float _speed;
		float _duration;
		float _elapsed{};
		EaseFn _curve;		// 기본값 Reverse: 1에서 0으로 감쇠
	};
}
