#include<conio.h>

import std;
import Warmup1;


matrix4x4 A;
matrix4x4 B;

int InputHandler(char input);

int main()
{
	wu1::RandomMatrix(A);
	wu1::RandomMatrix(B);

	char input;
	while (true)
	{
		std::system("cls");
		wu1::PrintMatrix(A, B);

		std::cout << R"(
  · m : 행렬의 곱셈
  · a : 행렬의 덧셈
  · d : 행렬의 뺄셈
  · r : 행렬식의 값(Determinant) → 입력한 2개의 행렬의 행렬식 값을 모두 출력한다.
  · t : 전치 행렬(Transposed matrix)과 그 행렬식의 값, 설정된 두 개의 행렬에 모두 적용한다.
  · e : 각 행에서 최소값을 찾아 그 값을 해당 행의 값에서 뺀다. → 다시 누르면 원래대로 출력한다.
  · f : 각 열에서 최대값을 찾아 그 값을 해당 열의 값에 더한다. → 다시 누르면 원래대로 출력한다.
  · + / - : 행렬의 모든 값에 +1 / -1 을 진행하고, 값의 범위는 10으로 모듈러 연산하여 0~9 로 한다. (8 ↔ 9 ↔ 0 ↔ 1 …)
  · s : 행렬의 값을 새로 랜덤하게 설정한다.
  · q : 프로그램 종료
)" << std::endl;


		if (!(std::cin >> input))
			return 0;

		if (InputHandler(input) == -1)
			return 0;

		std::cout << "계속하려면 아무 키나 입력하세요..";
		_getch();
	}
}

int InputHandler(char input)
{
	std::optional<matrix4x4> result1{}, result2{};
	std::optional<int> detA{}, detB{};
	switch (input)
	{
	case 'm':
		result1 = wu1::MulMatrix(A, B);
		break;
	case 'a':
		result1 = wu1::AddMatrix(A, B);
		break;
	case 'd':
		result1 = wu1::SubMatrix(A, B);
		break;
	case 'r':
		detA = wu1::DetMatrix(A);
		detB = wu1::DetMatrix(B);
		break;
	case 't':
		result1 = wu1::TMatrix(A);
		result2 = wu1::TMatrix(B);
		break;
	case 'e':
		result1 = wu1::SubRowMin(A);
		result2 = wu1::SubRowMin(B);
		break;
	case 'f':
		result1 = wu1::AddColMax(A);
		result2 = wu1::AddColMax(B);
		break;
	case '+':
		result1 = wu1::AddOneMatrix(A);
		result2 = wu1::AddOneMatrix(B);
		break;
	case '-':
		result1 = wu1::SubOneMatrix(A);
		result2 = wu1::SubOneMatrix(B);
		break;
	case 's':
		wu1::RandomMatrix(A);
		wu1::RandomMatrix(B);
		return 0;
	case 'q':
	default:
		return -1;
	}
	// 결과 출력
	std::cout << "결과" << std::endl;
	if (result1)
	{
		if (result2)
			wu1::PrintMatrix(*result1, *result2);
		else
			wu1::PrintMatrix(*result1);
	}
	else if (detA)
	{
		if (detB)
		{
			std::cout << *detA << " | " << *detB << std::endl;
		}
	}
	
	return 0;
}
