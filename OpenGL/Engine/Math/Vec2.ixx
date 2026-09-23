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
}
