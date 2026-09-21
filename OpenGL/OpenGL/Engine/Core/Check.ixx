module;
#include <intrin.h>		// __debugbreak

export module hs.check;

import std;

namespace hs
{
	[[noreturn]] void Fail(std::string_view message, const std::source_location& where)
	{
		std::println(std::cerr, "[Check] {}\n  {}({}) {}",
			message, where.file_name(), where.line(), where.function_name());
		std::cerr.flush();

		__debugbreak();
		std::abort();
	}
}

// Assert 대용
export namespace hs
{
#ifdef NDEBUG
	inline constexpr bool DebugChecks = false;
#else
	inline constexpr bool DebugChecks = true;
#endif

	// 조건식은 Release에서도 계산된다. 비교 한두 개짜리 검사용
	constexpr void Check(bool condition, std::string_view message = "Check failed",
		std::source_location where = std::source_location::current())
	{
		if constexpr (DebugChecks)
		{
			if (!condition)
				Fail(message, where);
		}
	}

	// 조건식이 Release에서 통째로 사라진다. 계산이 비싼 검사는 람다로
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
