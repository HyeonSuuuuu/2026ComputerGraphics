// std::println 대신 cout: 모듈에서 std::format 계열을 직접 쓰면 GCC 링크 충돌 (hs.text 주석)
export module engine_test.expect;

import std;

namespace engine_test
{
	int failures = 0;
}

export namespace engine_test
{
	void Expect(bool ok, std::string_view what)
	{
		if (!ok)
			++failures;
		std::cout << "  " << (ok ? "O" : "X") << ' ' << what << '\n';
	}

	int Failures() { return failures; }
}
