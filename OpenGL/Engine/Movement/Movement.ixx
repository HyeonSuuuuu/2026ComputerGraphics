export module hs.movement;

import std;
export import hs.transform;

// 위치·속도. 충돌이 바꾸는 값
export namespace hs
{
	struct Velocity
	{
		Vec2  dir{ 1.f, 0.f };	// 단위 벡터
		float speed{};

		Vec2 Value() const { return dir * speed; }
	};
	
	bool IsValidDirection(Vec2 dir)
	{
		float lenSq = LengthSq(dir);
		return lenSq == 0.f || std::abs(lenSq - 1.f) < 1e-3f;
	}

	enum class BoundsResponse
	{
		None,
		Reflect,
		Wrap,
		Clamp,
	};

	// 월드 경계 대신 이 범위에 가둠(반응은 Mover::boundsResponse). Mover가 있어야 적용
	struct Confine
	{
		Bounds bounds;
	};
	
	class IMovementMode
	{
	public:
		virtual ~IMovementMode() = default;

		virtual void CalcVelocity(Velocity& velocity, const Transform& transform, float dt) = 0;
	};

	struct Mover
	{
		Mover() = default;
		explicit Mover(Velocity velocity, BoundsResponse response = BoundsResponse::Reflect)
			: velocity(velocity), boundsResponse(response) {}

		Velocity velocity;
		BoundsResponse boundsResponse{ BoundsResponse::Reflect };
		bool enabled{ true };
		bool hitBounds{};	// 이번 CollisionSystem::Tick에서 경계에 걸림(밀림·튕김·넘어감). 매 Tick 새로 씀

		std::unique_ptr<IMovementMode> mode;

		void SetMode(std::unique_ptr<IMovementMode> newMode) { mode = std::move(newMode); }
	};
}
