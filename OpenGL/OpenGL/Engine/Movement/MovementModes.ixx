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

		void CalcVelocity(Velocity& velocity, const Transform&, float dt) override
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

	// 목표를 읽어오는 함수. 캡처한 대상이 이 모드보다 오래 살아야 한다
	using TargetFn = std::function<Vec2()>;

	// 목표 지점을 향한다. 도착 반경 안에 들어오면 멈춘다
	class FollowMode final : public IMovementMode
	{
	public:
		explicit FollowMode(TargetFn target, float arriveRadius = 0.01f)
			: _target(std::move(target)), _arriveRadius(arriveRadius)
		{
			Check(_target != nullptr, "목표를 읽을 방법이 없다");
			Check(arriveRadius >= 0.f);
		}

		explicit FollowMode(Vec2 target, float arriveRadius = 0.01f)
			: FollowMode(TargetFn{ [target] { return target; } }, arriveRadius) {}

		void CalcVelocity(Velocity& velocity, const Transform& transform, float) override
		{
			Vec2 offset = _target() - transform.pos;
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
		TargetFn _target;
		float _arriveRadius;
		bool _reached{};
	};
}
