export module hs.text;

import std;

namespace hs
{
	std::string VFormat(std::string_view format, std::format_args args)
	{
		return std::vformat(format, args);
	}
}

// std::format·println을 여러 모듈에서 직접 쓰면 GCC 16 링크 충돌(format 내부 정적 데이터가 모듈마다 생김)
// → 문자열을 만드는 일은 이 모듈 한 곳에서만. 부르는 쪽은 인자 포장만 하고, 형식 검사는 여전히 컴파일 타임
export namespace hs
{
	template<class... Args>
	std::string Format(std::format_string<Args...> format, Args&&... args)
	{
		return VFormat(format.get(), std::make_format_args(args...));
	}
}
