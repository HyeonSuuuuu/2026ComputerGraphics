export module hs.movement_modes;

import std;
import hs.check;
export import hs.transform;
export import hs.movement;

export namespace hs
{
	// mode 없음(nullptr) = 등속 직선

	// 가로 진행·경계 반사는 시스템 담당
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

	// 시계방향. s 간격을 벌려두면 줄지어 이동
	class EdgePatrolMode final : public IMovementMode
	{
	public:
		EdgePatrolMode(Bounds area, float speed)
			: _area(area), _speed(speed)
		{
			Check(speed > 0.f, "속도가 0이면 제자리에 선다");
			Check(Perimeter(area) > 0.f, "영역이 한 점이면 돌 곳이 없다");
		}

		static float Perimeter(const Bounds& area)
		{
			Vec2 size = area.Size();
			return 2.f * (size.x + size.y);
		}

		void CalcVelocity(Velocity& velocity, const Transform& transform, float dt) override
		{
			if (dt <= 0.f)
				return;

			// 벽 접촉 전엔 기존 이동, 닿은 자리에서 테두리 합류
			if (!_joined)
			{
				if (!OnBorder(transform.pos))
					return;

				_s = SFromPoint(transform.pos);
				_joined = true;
			}

			_s = std::fmod(_s + _speed * dt, Perimeter(_area));

			// 목표 지점까지의 속도 역산
			Vec2 offset = PointAt(_s) - transform.pos;
			velocity.dir = Normalize(offset);
			velocity.speed = Length(offset) / dt;
		}

	private:
		bool OnBorder(Vec2 pos) const
		{
			return pos.x <= _area.min.x + Epsilon || pos.x >= _area.max.x - Epsilon
				|| pos.y <= _area.min.y + Epsilon || pos.y >= _area.max.y - Epsilon;
		}

		// PointAt의 역
		float SFromPoint(Vec2 pos) const
		{
			Vec2 size = _area.Size();
			float toTop = _area.max.y - pos.y;
			float toRight = _area.max.x - pos.x;
			float toBottom = pos.y - _area.min.y;
			float toLeft = pos.x - _area.min.x;
			float nearest = std::min({ toTop, toRight, toBottom, toLeft });

			if (nearest == toTop)		return pos.x - _area.min.x;
			if (nearest == toRight)		return size.x + (_area.max.y - pos.y);
			if (nearest == toBottom)	return size.x + size.y + (_area.max.x - pos.x);
			return 2.f * size.x + size.y + (pos.y - _area.min.y);
		}

		// 좌상단 출발, 시계방향
		Vec2 PointAt(float s) const
		{
			Vec2 size = _area.Size();

			if (s < size.x)			return { _area.min.x + s, _area.max.y };		// 위: 왼→오
			s -= size.x;
			if (s < size.y)			return { _area.max.x, _area.max.y - s };		// 오른쪽: 위→아래
			s -= size.y;
			if (s < size.x)			return { _area.max.x - s, _area.min.y };		// 아래: 오→왼
			s -= size.x;
			return { _area.min.x, _area.min.y + s };							// 왼쪽: 아래→위
		}

		Bounds _area;
		float _speed;
		float _s{};
		bool _joined{};

		static constexpr float Epsilon = 1e-4f;
	};

	// 수명 주의: 캡처 대상이 이 모드보다 오래 생존
	using TargetFn = std::function<Vec2()>;

	// 도착 반경 안에서 정지
	class FollowMode final : public IMovementMode
	{
	public:
		explicit FollowMode(TargetFn target, float arriveRadius = 0.01f)
			pre (arriveRadius >= 0.f)
			: _target(std::move(target)), _arriveRadius(arriveRadius)
		{
			Check(_target != nullptr, "목표를 읽을 방법이 없다");
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
