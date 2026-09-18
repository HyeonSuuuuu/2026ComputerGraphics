module;
#include <intrin.h>		// __debugbreak

export module hs.check;

import std;

export namespace hs
{
#ifdef NDEBUG
	inline constexpr bool DebugChecks = false;
#else
	inline constexpr bool DebugChecks = true;
#endif

	[[noreturn]] void Fail(std::string_view message, const std::source_location& where)
	{
		std::println(std::cerr, "[Check] {}\n  {}({}) {}",
			message, where.file_name(), where.line(), where.function_name());
		std::cerr.flush();

		__debugbreak();
		std::abort();
	}
	// Release에서 조건값 계산 안함 대신 람다 써야함
	template<bool Enabled = DebugChecks, std::predicate C>
	constexpr void Check(C cond, std::string_view message = "Check failed",
		std::source_location where = std::source_location::current())
	{
		if constexpr (Enabled)
		{
			if (!cond())
				Fail(message, where);
		}
	}
	// Release에서 조건값은 계산됨 (가벼운 계산이라 큰상관 없음)
	template<bool Enabled = DebugChecks>
	constexpr void Check(bool condition, std::string_view message = "Check failed",
		std::source_location where = std::source_location::current())
	{
		if constexpr (Enabled)
		{
			if (!condition)
				Fail(message, where);
		}
	}
}
