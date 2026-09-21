module;
#include <gl/glm/glm.hpp>

export module hs.movement;

import std;
import hs.shape;

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
	
	enum class MixMode
	{
		Override,
		Additive,
	};

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

	// 기본 이동 위에 잠깐 얹히는 한 겹. 이번 프레임의 속도만 계산하고, 끝나면 제거된다
	class ILayeredMove
	{
	public:
		virtual ~ILayeredMove() = default;

		virtual MixMode GetMixMode() const = 0;
		virtual Vec2 Evaluate(const Transform& transform, float dt) = 0;
		virtual bool IsFinished() const = 0;
	};

	// 오브젝트에 붙이는 이동 컴포넌트. 데이터만 들고 있고 실행은 MovementSystem이 한다
	struct Mover
	{
		Mover() = default;
		explicit Mover(Velocity velocity, BoundsResponse bounds = BoundsResponse::Reflect)
			: velocity(velocity), bounds(bounds) {}

		Velocity velocity;
		BoundsResponse bounds{ BoundsResponse::Reflect };
		bool enabled{ true };

		std::unique_ptr<IMovementMode> mode;
		std::vector<std::unique_ptr<ILayeredMove>> layeredMoves;

		// 이동 방식은 하나만 유효하다. 새로 넣으면 이전 것은 사라진다
		void SetMode(std::unique_ptr<IMovementMode> newMode) { mode = std::move(newMode); }

		// 잠깐 얹히는 이동. 끝나면 MovementSystem이 제거한다
		void AddLayeredMove(std::unique_ptr<ILayeredMove> move) { layeredMoves.push_back(std::move(move)); }
	};
}
