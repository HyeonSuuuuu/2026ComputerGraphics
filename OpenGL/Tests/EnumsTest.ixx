export module engine_test.enums;

import std;
import hs.enums;
import engine_test.expect;

using namespace hs;
using namespace engine_test;

namespace
{
	enum class Motion { Diagonal, ZigZag, EdgePatrol, Home };
	enum class Key : int { A = 65, B = 66, Escape = 256 };		// 값이 0, 1, 2…가 아님
}

export void RunEnumsTests()
{
	std::cout << "[Enums]\n";
	Expect(EnumCount<Motion> == 4 && EnumCount<Key> == 3, "EnumCount");
	Expect(EnumToString(Motion::ZigZag) == "ZigZag", "EnumToString");
	Expect(EnumToString(Key::Escape) == "Escape", "값이 256이어도 이름");
	Expect(EnumToString(static_cast<Key>(999)).empty(), "없는 값은 빈 문자열");
	Expect(StringToEnum<Motion>("Home") == Motion::Home && StringToEnum<Key>("B") == Key::B, "StringToEnum");
	Expect(!StringToEnum<Motion>("Jump"), "없는 이름은 nullopt");

	constexpr std::string_view atCompileTime = EnumToString(Motion::Home);
	Expect(atCompileTime == "Home", "컴파일 타임에도 동작");
}
