export module engine_test.type_id;

import std;
import hs.type_id;
import engine_test.expect;
import engine_test.components;

using namespace hs;
using namespace engine_test;

export void RunTypeIdTests()
{
	std::cout << "[TypeId]\n";
	Expect(TypeName<Home> == "engine_test::Home", "TypeName: 네임스페이스 포함");
	Expect(ShortTypeName<Home> == "Home", "ShortTypeName: 이름만 (창 제목용)");
	Expect(TypeIdOf<Home> != TypeIdOf<Speed>, "타입이 다르면 ID도 다름");
}
