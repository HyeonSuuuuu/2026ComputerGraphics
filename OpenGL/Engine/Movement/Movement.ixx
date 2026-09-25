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

		std::unique_ptr<IMovementMode> mode;

		void SetMode(std::unique_ptr<IMovementMode> newMode) { mode = std::move(newMode); }
	};
}
