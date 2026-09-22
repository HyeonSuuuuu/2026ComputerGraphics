module;
#include <gl/glm/glm.hpp>

export module hs.movement;

import std;
import hs.shape;
import hs.component;

// 위치와 속도를 다룬다. 충돌이 이 값들을 바꾼다
export namespace hs
{
	struct Velocity
	{
		Vec2  dir{ 1.f, 0.f };	// 정규화된 방향
		float speed{};

		Vec2 Value() const { return dir * speed; }
	};
	
	bool IsValidDirection(Vec2 dir)
	{
		float lenSq = LengthSq(dir);
		return lenSq == 0.f || std::abs(lenSq - 1.f) < 1e-3f;
	}

	// 경계에 닿았을 때의 처리
	enum class BoundsResponse
	{
		None,
		Reflect,
		Wrap,
		Clamp,
	};
	
	class IMovementMode
	{
	public:
		virtual ~IMovementMode() = default;

		virtual void CalcVelocity(Velocity& velocity, const Transform& transform, float dt) = 0;
	};

	// 오브젝트에 붙이는 이동 컴포넌트. 데이터만 들고 있고 실행은 MovementSystem이 한다
	struct Mover : IComponent
	{
		Mover() = default;
		explicit Mover(Velocity velocity, BoundsResponse response = BoundsResponse::Reflect)
			: velocity(velocity), boundsResponse(response) {}

		Velocity velocity;
		BoundsResponse boundsResponse{ BoundsResponse::Reflect };
		bool enabled{ true };

		std::unique_ptr<IMovementMode> mode;

		// 이동 방식은 하나만 유효하다. 새로 넣으면 이전 것은 사라진다
		void SetMode(std::unique_ptr<IMovementMode> newMode) { mode = std::move(newMode); }
	};
}
