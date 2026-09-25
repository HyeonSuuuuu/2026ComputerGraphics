export module hs.enums;

import std;

// 리플렉션으로 열거값 목록을 받아 반복을 펼침 → switch를 손으로 관리하지 않음. 실행 비용은 손으로 쓴 switch와 같음
export namespace hs
{
	template<class E> requires std::is_enum_v<E>
	constexpr std::size_t EnumCount = std::meta::enumerators_of(^^E).size();

	// 목록에 없는 값이면 빈 문자열. 값이 같은 열거값이 여럿이면 먼저 선언된 이름
	template<class E> requires std::is_enum_v<E>
	constexpr std::string_view EnumToString(E value)
	{
		template for (constexpr std::meta::info e : std::define_static_array(std::meta::enumerators_of(^^E)))
		{
			if (value == [: e :])
				return std::meta::identifier_of(e);
		}
		return {};
	}

	// 설정 파일 등에서 이름으로 읽을 때. 순서·값이 바뀌어도 이름만 같으면 됨
	template<class E> requires std::is_enum_v<E>
	constexpr std::optional<E> StringToEnum(std::string_view name)
	{
		template for (constexpr std::meta::info e : std::define_static_array(std::meta::enumerators_of(^^E)))
		{
			if (name == std::meta::identifier_of(e))
				return [: e :];
		}
		return std::nullopt;
	}
}
