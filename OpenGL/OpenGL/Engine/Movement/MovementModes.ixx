module;
#include <gl/glm/glm.hpp>

export module hs.movement_modes;

import std;
import hs.check;
import hs.shape;
import hs.movement;

export namespace hs
{
	// mode가 없으면(nullptr) 속도가 그대로 유지된다 = 등속 직선 이동

	// 일정 시간마다 세로 방향을 뒤집는다. 가로 진행과 경계 반사는 시스템이 처리
	class ZigZagMode final : public IMovementMode
	{
	public:
		explicit ZigZagMode(float interval) : _interval(interval)
		{
			Check(interval > 0.f, "간격이 0 이하면 방향이 매 프레임 뒤집힌다");
		}

		void CalcVelocity(Velocity& velocity, const Rectangle&, float dt) override
		{
			if (_interval <= 0.f)
				return;

			_timer += dt;
			while (_timer >= _interval)
			{
				_timer -= _interval;
				velocity.dir.y = -velocity.dir.y;
			}
		}

	private:
		float _interval;
		float _timer{};
	};

	// 목표 지점을 향한다. 도착 반경 안에 들어오면 멈춘다
	class FollowMode final : public IMovementMode
	{
	public:
		FollowMode(Vec2 target, float arriveRadius = 0.01f)
			: _target(target), _arriveRadius(arriveRadius)
		{
			Check(arriveRadius >= 0.f);
		}

		void SetTarget(Vec2 target) { _target = target; }
		Vec2 GetTarget() const { return _target; }

		void CalcVelocity(Velocity& velocity, const Rectangle& rect, float) override
		{
			Vec2 offset = _target - rect.pos;
			if (LengthSq(offset) <= _arriveRadius * _arriveRadius)
			{
				_reached = true;
				velocity.speed = 0.f;
				return;
			}

			_reached = false;
			velocity.dir = Normalize(offset);
		}

		bool Reached() const { return _reached; }

	private:
		Vec2 _target;
		float _arriveRadius;
		bool _reached{};
	};
}
