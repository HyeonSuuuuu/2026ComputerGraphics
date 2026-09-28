// 이 모듈과 엔진(Check.ixx)이 둘 다 Format을 쓰는데도 링크되는 것 자체가 검사의 절반
export module engine_test.text;

import std;
import hs.text;
import engine_test.expect;

using namespace hs;
using namespace engine_test;

export void RunTextTests()
{
	std::cout << "[Text]\n";
	Expect(Format("{} + {} = {}", 1, 2.5f, "셋") == "1 + 2.5 = 셋", "Format: 정수·실수·문자열");
	Expect(Format("{:>4}|", 7) == "   7|", "Format: 폭 지정");
}
