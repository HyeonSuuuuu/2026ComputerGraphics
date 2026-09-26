// 종료 코드 = 실패 수. 새 테스트 묶음은 모듈로 만들고 여기서 부름

import std;
import engine_test.expect;
import engine_test.world;
import engine_test.enums;
import engine_test.fields;
import engine_test.text;
import engine_test.type_id;

int main()
{
	RunWorldTests();
	RunEnumsTests();
	RunFieldsTests();
	RunTextTests();
	RunTypeIdTests();

	int failures = engine_test::Failures();
	std::cout << '\n' << (failures == 0 ? "전부 통과" : "실패 있음") << '\n';
	return failures;
}
