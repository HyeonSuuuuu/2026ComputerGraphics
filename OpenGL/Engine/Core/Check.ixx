module;
#include <contracts>

export module hs.check;

import std;

// 컨트랙트(pre·contract_assert) 위반 시 표준 기본 처리 대신 호출됨. 전역 모듈에 붙여야 교체됨
// 줄 번호는 pre를 적은 쪽(부른 쪽 아님) → 부른 곳은 디버거 호출 스택으로
extern "C++" void handle_contract_violation(const std::contracts::contract_violation& violation)
{
	std::println(std::cerr, "[Contract] {}\n  {}({})",
		violation.comment(), violation.location().file_name(), violation.location().line());
	std::cerr.flush();

	std::breakpoint_if_debugging();
	if (violation.is_terminating())
		std::abort();	// 돌아가면 terminate가 "active exception 없음"이라는 엉뚱한 메시지를 냄
}

namespace hs
{
	[[noreturn]] void Fail(std::string_view message, const std::source_location& where)
	{
		std::println(std::cerr, "[Check] {}\n  {}({}) {}",
			message, where.file_name(), where.line(), where.function_name());
		std::cerr.flush();

		std::breakpoint_if_debugging();
		std::abort();
	}
}

export namespace hs
{
#ifdef NDEBUG
	inline constexpr bool DebugChecks = false;
#else
	inline constexpr bool DebugChecks = true;
#endif

	// Release에서도 조건식 계산. 가벼운 비교용
	constexpr void Check(bool condition, std::string_view message = "Check failed",
		std::source_location where = std::source_location::current())
	{
		if constexpr (DebugChecks)
		{
			if (!condition)
				Fail(message, where);
		}
	}

	// Release에서 조건식 통째로 제거. 비싼 검사용
	template<std::predicate C>
	constexpr void CheckSlow(C cond, std::string_view message = "Check failed",
		std::source_location where = std::source_location::current())
	{
		if constexpr (DebugChecks)
		{
			if (!cond())
				Fail(message, where);
		}
	}
}
