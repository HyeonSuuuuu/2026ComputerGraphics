module;
#define GLM_FORCE_CTOR_INIT // 기본 생성자 0 초기화
#include <glm/glm.hpp>

export module hs.vec2;

// glm 연산자는 전역 모듈 조각에만 있어 import한 쪽에서 안 보임 → 함께 내보냄
export namespace glm
{
	using glm::operator+;
	using glm::operator-;
	using glm::operator*;
	using glm::operator/;
	using glm::operator==;
	using glm::operator!=;
}

export namespace hs
{
	using Vec2 = glm::vec2;

	float LengthSq(Vec2 v) { return glm::dot(v, v); }
	float Length(Vec2 v) { return glm::length(v); }
	Vec2 Abs(Vec2 v) { return glm::abs(v); }

	Vec2 Normalize(Vec2 v)
	{
		return LengthSq(v) > 0.f ? glm::normalize(v) : Vec2{};
	}

	// 목표를 향해 매 프레임 남은 거리의 rate·dt만큼. 목표가 바뀌어도 이어지고, 큰 dt에서도 넘치지 않음
	Vec2 Approach(Vec2 current, Vec2 target, float rate, float dt)
		pre (rate >= 0.f && dt >= 0.f)
	{
		return current + (target - current) * glm::min(rate * dt, 1.f);
	}
}
