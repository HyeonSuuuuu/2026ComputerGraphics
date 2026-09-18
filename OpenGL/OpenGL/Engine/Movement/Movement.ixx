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

	// dir은 정규화되어 있거나 0이어야 한다. assert에서 쓴다
	bool IsValidDirection(Vec2 dir)
	{
		float lenSq = LengthSq(dir);
		return lenSq == 0.f || std::abs(lenSq - 1.f) < 1e-3f;
	}

	// LayeredMove를 최종 속도에 합치는 방식
	enum class MixMode
	{
		Override,
		Additive,
	};

	// 경계에 닿았을 때의 처리
	enum class BoundsResponse
	{
		None,
		Reflect,	// 반사
		Wrap,		// 반대편으로
		Clamp,		// 경계에서 멈춤
	};
	
	// 이동 방식. 한 번에 하나만 활성화
	// Velocity를 수정할 뿐 위치는 건드리지 않는다
	class IMovementMode
	{
	public:
		virtual ~IMovementMode() = default;

		virtual void CalcVelocity(Velocity& velocity, const Rectangle& rect, float dt) = 0;
	};

	// 기본 이동 위에 잠깐 얹히는 한 겹. 이번 프레임의 속도만 계산하고, 끝나면 제거된다
	class ILayeredMove
	{
	public:
		virtual ~ILayeredMove() = default;

		virtual MixMode Mode() const = 0;
		virtual Vec2 Evaluate(const Rectangle& rect, float dt) = 0;
		virtual bool IsFinished() const = 0;
	};
}
