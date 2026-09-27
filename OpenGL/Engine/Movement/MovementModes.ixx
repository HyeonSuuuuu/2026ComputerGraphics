module;
#include "Core/Check.h"

export module hs.movement_modes;

import std;
import hs.check;
export import hs.transform;
export import hs.movement;

export namespace hs
{
	// mode 없음(nullptr) = 등속 직선

	// 잔디 깎기: 가로로 벽까지 → step만큼 세로로 → 반대 가로로 벽까지 → … 위·아래 벽에 닿으면 세로 방향을 뒤집음
	// 벽 위치는 모름. BoundsResponse::Reflect가 뒤집은 방향 부호로 "닿았다"를 앎 → Reflect와 함께 쓸 것
	class SweepMode final : public IMovementMode
	{
	public:
		explicit SweepMode(float step, float verticalSign = -1.f) : _step(step), _verticalSign(verticalSign)
		{
			HS_DCHECK(step > 0.f, "한 칸이 0이면 세로로 안 움직인다");
		}

		void CalcVelocity(Velocity& velocity, const Transform&, float dt) override
		{
			if (!_started) {
				_horizontalSign = velocity.dir.x < 0.f ? -1.f : 1.f;
				velocity.dir = { _horizontalSign, 0.f };
				_started = true;
				return;
			}

			if (!_stepping) {
				if (velocity.dir.x * _horizontalSign < 0.f) {	// 옆 벽에서 반사됨
					_horizontalSign = -_horizontalSign;
					_stepping = true;
					_stepLeft = _step;
					velocity.dir = { 0.f, _verticalSign };
				}
				return;
			}

			if (velocity.dir.y * _verticalSign < 0.f) {			// 위·아래 벽에서 반사됨 → 이후로는 반대로
				_verticalSign = -_verticalSign;
				_stepLeft = 0.f;
			}
			else
				_stepLeft -= velocity.speed * dt;

			if (_stepLeft <= 0.f) {
				_stepping = false;
				velocity.dir = { _horizontalSign, 0.f };
			}
		}

	private:
		float _step;
		float _verticalSign;
		float _horizontalSign = 1.f;
		float _stepLeft{};
		bool _stepping{};
		bool _started{};
	};

	// 점들을 직선으로 이은 경로를 speed로 따라감. 끝에 닿으면 되돌아옴(왕복)
	// 경로 첫 점에서 시작해야 튀지 않음. 속력은 모드가 정함 → 다른 모드로 바꿀 때 speed를 다시 줄 것
	class PathMode final : public IMovementMode
	{
	public:
		PathMode(std::vector<Vec2> points, float speed) : _points(std::move(points)), _speed(speed)
		{
			HS_DCHECK(_points.size() >= 2, "점이 둘은 있어야 경로");
			HS_DCHECK(speed > 0.f, "속도가 0이면 제자리에 선다");
			_lengths.push_back(0.f);	// _lengths[i] = 처음부터 i번 점까지 거리
			for (std::size_t i = 1; i < _points.size(); ++i)
				_lengths.push_back(_lengths.back() + Length(_points[i] - _points[i - 1]));
		}

		void CalcVelocity(Velocity& velocity, const Transform& transform, float dt) override
		{
			if (dt <= 0.f)
				return;

			const float total = _lengths.back();
			_s += _speed * dt * (_forward ? 1.f : -1.f);
			// 끝을 넘어간 만큼은 되돌아온 거리로. 끝에 세워 두면 왕복마다 한 프레임씩 늦어짐
			if (_s > total) {
				_s = std::max(2.f * total - _s, 0.f);
				_forward = false;
			}
			else if (_s < 0.f) {
				_s = std::min(-_s, total);
				_forward = true;
			}

			// 다음 점까지의 속도 역산 (EdgePatrolMode와 같은 방식) → 꺾이는 점을 지나쳐도 경로 위
			Vec2 offset = PointAt(_s) - transform.pos;
			velocity.dir = Normalize(offset);
			velocity.speed = Length(offset) / dt;
		}

	private:
		Vec2 PointAt(float s) const
		{
			auto next = std::upper_bound(_lengths.begin(), _lengths.end(), s);
			if (next == _lengths.end())
				return _points.back();
			std::size_t i = static_cast<std::size_t>(next - _lengths.begin());	// s는 i-1번과 i번 점 사이
			float segment = _lengths[i] - _lengths[i - 1];
			float t = segment > 0.f ? (s - _lengths[i - 1]) / segment : 0.f;
			return _points[i - 1] + (_points[i] - _points[i - 1]) * t;
		}

		std::vector<Vec2> _points;
		std::vector<float> _lengths;
		float _speed;
		float _s{};
		bool _forward = true;
	};

	// 시계방향. s 간격을 벌려두면 줄지어 이동
	class EdgePatrolMode final : public IMovementMode
	{
	public:
		EdgePatrolMode(Bounds area, float speed)
			: _area(area), _speed(speed)
		{
			HS_DCHECK(speed > 0.f, "속도가 0이면 제자리에 선다");
			HS_DCHECK(Perimeter(area) > 0.f, "영역이 한 점이면 돌 곳이 없다");
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
			: _target(std::move(target)), _arriveRadius(arriveRadius)
		{
			HS_DCHECK(_target != nullptr, "목표를 읽을 방법이 없다");
			HS_DCHECK(arriveRadius >= 0.f, "도착 반경이 음수면 영영 도착하지 않는다");
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
