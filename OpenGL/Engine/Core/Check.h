// 단언: HS_DCHECK(조건, "무엇이 왜 틀렸는지") — D = Debug 전용 (Abseil·Chromium 관례: CHECK는 항상, DCHECK는 Debug만)
// Release(NDEBUG)에선 조건식 글자째 사라짐 → 계산 비용 0, 람다 불필요. 실패 시 조건식도 함께 출력
// 사라지므로 조건식에 부작용 금지 (HS_DCHECK(Pop() != 0, …)는 Release에서 Pop이 안 불림)
// 매크로는 모듈로 내보낼 수 없어 헤더. 쓰는 모듈의 전역 모듈 조각(module; 아래)에서 include하고, hs.check를 import
// CMake가 헤더 유닛을 지원하면 import "Core/Check.h";로 바꿀 수 있음 (CMake 4.4 기준 미지원)
// 주의: 조건에 쉼표가 있는 템플릿(Get<A, B>)은 괄호로 한 번 더 감쌀 것
#pragma once

#ifdef NDEBUG
	#define HS_DCHECK(condition, message) ((void)0)
#else
	#define HS_DCHECK(condition, message) \
		((condition) ? (void)0 : ::hs::CheckFailed(#condition, message, ::std::source_location::current()))
#endif
