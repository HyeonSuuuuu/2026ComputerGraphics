export module engine_test.fields;

import std;
import hs.fields;
import engine_test.expect;

using namespace hs;
using namespace engine_test;

namespace
{
	enum class Motion { Diagonal, ZigZag };

	struct Status
	{
		Motion motion = Motion::ZigZag;
		bool moving = true;
		float speed = 0.5f;
	};

	struct Empty {};
}

export void RunFieldsTests()
{
	std::cout << "[Fields]\n";
	Expect(Describe(Status{}) == "motion=ZigZag moving=true speed=0.5", "Describe: 선언 순서, enum은 이름");
	Expect(Describe(Empty{}).empty(), "필드 없으면 빈 문자열");
}
