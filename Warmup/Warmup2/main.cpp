#include<conio.h>

import std;
import Warmup2;

using Line = std::vector<std::string>;
using Lines = std::vector<Line>;


// ===== 공통 출력 =====

void PrintAll(const Lines& lines, auto printLine)
{
	for (const auto& line : lines)
	{
		printLine(line);
		std::cout << std::endl;
	}
}

void PrintPlain(const Lines& lines)
{
	PrintAll(lines, [](const Line& line) { wu2::PrintLine(line); });
}

void PauseAndClear()
{
	std::cout << "아무 키나 입력세요..";
	_getch();
	std::system("cls");
}


// ===== 명령별 출력 =====

void PrintWithCounts(const Lines& lines)
{
	const auto counts = wu2::GetWordCount(lines);
	for (int i = 0; const auto& line : lines)
	{
		wu2::PrintLine(line);
		if (counts)
			std::cout << " " << (*counts)[i];
		std::cout << std::endl;
		++i;
	}
}

void ColorBlock(const Lines& lines)
{
	PrintAll(lines, [](const Line& line) { wu2::PrintColorLine(line); });
	std::cout << "총 개수: " << wu2::colorCount << std::endl;
	wu2::colorCount = 0;
	PauseAndClear();
}

void ReverseBlock(const Lines& lines)
{
	PrintAll(lines, [](const Line& line) { wu2::PrintReverseLine(line); });
	PauseAndClear();
}

void StarBlock(const Lines& lines)
{
	PrintAll(lines, [](const Line& line) { wu2::PrintStarLine(line); });
	PauseAndClear();
}

void ReverseWordBlock(const Lines& lines)
{
	PrintAll(lines, [](const Line& line) { wu2::PrintReverseWordLine(line); });
	PauseAndClear();
}

void ChangeBlock(const Lines& lines)
{
	char from, to;
	std::cout << "바꿀 문자를 입력하세요";
	std::cin >> from;
	std::cout << "새로운 문자를 입력하세요";
	std::cin >> to;

	PrintAll(lines, [from, to](const Line& line) { wu2::PrintChangeLine(line, from, to); });
	PauseAndClear();
}

void NumberBlock(const Lines& lines)
{
	// PrintNumberLine 은 뒤의 두 인자를 쓰지 않는다 (원본에서는 미초기화 값이 넘어갔다)
	PrintAll(lines, [](const Line& line) { wu2::PrintNumberLine(line, char{}, char{}); });
	PauseAndClear();
}

void ColorWordBlock(const Lines& lines)
{
	std::string searchWord;
	std::cout << "찾을 단어를 입력하세요";
	std::cin >> searchWord;

	PrintAll(lines, [&searchWord](const Line& line) { wu2::PrintColorWordLine(line, searchWord); });
	std::cout << "총 개수: " << wu2::colorCount;
	wu2::colorCount = 0;
	PauseAndClear();
}

void ChangeLineBlock(const Lines& lines)
{
	bool firstFlag{};
	for (const auto& line : lines)
	{
		if (firstFlag)
			wu2::PrintLine(line);
		std::cout << std::endl;
		firstFlag = true;
	}
	wu2::PrintLine(lines[0]);
	std::cout << std::endl;
	PauseAndClear();
}


// ===== 명령 표 =====

struct Command
{
	char key;
	std::string_view help;
	void (*run)(Lines&);
};

const std::array<Command, 10> commands
{{
	{ 'a', "모든 문장의 문자들을 대소문자를 바꿔 출력한다. → 다시 누르면 원래대로 출력한다.",
	  [](Lines& lines) { wu2::TogleCase(lines); PrintPlain(lines); } },

	{ 'b', "각 줄의 단어의 개수를 출력한다. 문장들을 모두 쓰고 각 문장의 맨 뒤에 해당 문장의 단어의 개수를 출력한다.",
	  [](Lines& lines) { PrintWithCounts(lines); } },

	{ 'c', "대문자로 시작되는 단어를 찾아 그 단어를 다른 색으로 출력하고, 몇 개 있는지를 계산하여 출력한다. → 다시 누르면 원래대로 출력한다.",
	  [](Lines& lines) { ColorBlock(lines); PrintPlain(lines); } },

	{ 'd', "각 문장 별로 거꾸로 출력하기. → 다시 누르면 원래대로 출력한다.",
	  [](Lines& lines) { ReverseBlock(lines); PrintPlain(lines); } },

	{ 'e', "모든 공백에 “*” 문자 삽입하기. → 다시 누르면 원래대로 출력한다.",
	  [](Lines& lines) { StarBlock(lines); PrintPlain(lines); } },

	{ 'f', "공백을 기준으로 (*가 삽입되어 있다면 *를 공백으로 취급) 모든 단어들을 거꾸로 출력하기. → 다시 누르면 원래대로 출력한다.",
	  [](Lines& lines) { ReverseWordBlock(lines); PrintPlain(lines); } },

	{ 'g', "문자 내부의 특정 문자를 다른 문자로 바꾸기 (바꿀 문자와 새롭게 입력할 문자 입력 받음) → 다시 누르면 원래대로 출력한다.",
	  [](Lines& lines) { ChangeBlock(lines); PrintPlain(lines); } },

	{ 'h', "문장에 있는 숫자를 찾아 숫자 뒤에 오는 문장을 다음 줄로 넘긴다. → 다시 누르면 원래대로 출력한다.",
	  [](Lines& lines) { NumberBlock(lines); PrintPlain(lines); } },

	{ 'i', "명령어와 단어를 입력하면, 문장들을 모두 출력하면서 입력 받은 단어를 찾아 다른 색으로 출력하고, 몇 개 있는지를 계산하여 출력한다.\n(대소문자 구분하지 않는다)",
	  [](Lines& lines) { ColorWordBlock(lines); PrintPlain(lines); } },

	{ 'j', "문장의 순서를 바꿔서 출력한다. 즉, 1번 문장 → 2번 문장, 2번 문장 → 3번 문장, … , 9번 문장 → 1번 문장",
	  [](Lines& lines) { ChangeLineBlock(lines); PrintPlain(lines); } },
}};

void PrintMenu()
{
	std::cout << '\n';
	for (const auto& command : commands)
		std::cout << command.key << ": " << command.help << '\n';
	std::cout << "q: 프로그램 종료" << '\n' << std::endl;
}


int main()
{
	std::cout << "데이터 파일 이름: ";

	std::string fileName;
	if (!(std::cin >> fileName))
		return 0;

	auto lines = wu2::ReadLines(fileName);
	if (!lines)
	{
		std::cout << "파일을 열 수 없습니다: " << fileName << std::endl;
		return 1;
	}

	std::cout << "[단어 구분] 완료" << std::endl;

	while (true)
	{
		PrintMenu();

		char input;
		if (!(std::cin >> input))
			return 0;

		std::system("cls");

		const auto command = std::ranges::find(commands, input, &Command::key);
		if (command == commands.end())      // q 및 그 밖의 입력
			return 0;

		command->run(*lines);
	}
}