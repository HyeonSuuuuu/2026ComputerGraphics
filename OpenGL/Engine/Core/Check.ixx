export module hs.check;

import std;

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
