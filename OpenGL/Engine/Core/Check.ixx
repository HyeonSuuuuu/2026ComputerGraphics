export module hs.check;

import std;
import hs.text;

export namespace hs
{
	// HS_DCHECK 매크로(Check.h)가 실패했을 때만 부름
	[[noreturn]] void CheckFailed(std::string_view condition, std::string_view message, const std::source_location& where)
	{
		std::cerr << Format("[Check] {}\n  조건: {}\n  {}({}) {}\n",
			message, condition, where.file_name(), where.line(), where.function_name());
		std::cerr.flush();

		std::breakpoint_if_debugging();
		std::abort();
	}
}
