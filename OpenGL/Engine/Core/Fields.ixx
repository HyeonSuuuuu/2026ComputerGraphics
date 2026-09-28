export module hs.fields;

import std;
import hs.enums;
import hs.text;

export namespace hs
{
	// "이름=값 이름=값" 한 줄. 필드를 추가하면 따라옴. enum은 이름으로, 나머지는 Format이 찍을 수 있어야 함
	template<class T>
	std::string Describe(const T& object)
	{
		std::string out;
		template for (constexpr std::meta::info m : std::define_static_array(
			std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unchecked())))
		{
			using Field = std::remove_cvref_t<decltype(object.[: m :])>;

			if (!out.empty())
				out += ' ';
			if constexpr (std::is_enum_v<Field>)
				out += Format("{}={}", std::meta::identifier_of(m), EnumToString(object.[: m :]));
			else
				out += Format("{}={}", std::meta::identifier_of(m), object.[: m :]);
		}
		return out;
	}
}
