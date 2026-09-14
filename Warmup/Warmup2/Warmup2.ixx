module;

#include <windows.h>


export module Warmup2;

import std;

char ToggleChar(char c);


export namespace wu2
{
	int colorCount = 0;

	std::vector<std::string> SplitWords(const std::string& line)
	{
		auto words = line
			| std::views::split(' ')
			| std::views::filter([](auto r) {
			return !r.empty(); })
			| std::views::transform([](auto r) {
			return std::string(r.begin(), r.end()); })
			| std::ranges::to<std::vector>();

		return words;
	}

	std::optional<std::vector<std::vector<std::string>>> ReadLines(const std::filesystem::path& fileName)
	{
		std::ifstream file(fileName);
		if (!file)
			return std::nullopt;

		std::vector<std::vector<std::string>> lines;
		
		std::string line;
		while (std::getline(file, line))
		{
			lines.push_back(SplitWords(line));
		}
		return lines;
	}

	void PrintLine(const auto& line)
	{
		for (const auto& word : line)
		{
			std::cout << word << " ";
		}
	}

	void PrintColorLine(const auto& line)
	{
		for (const auto& word : line)
		{
			for (const auto& c : word)
			{
				if (std::isupper(c))
				{
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 9);
					++colorCount;
					std::cout << c;
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
				}
				else
				{
					std::cout << c;
				}
			}
			std::cout << " ";
		}
	}

	void PrintReverseLine(const auto& line)
	{
		for (const auto& word : std::views::reverse(line))
		{
			for (const auto& c : std::views::reverse(word))
			{
					std::cout << c;
			}
			std::cout << " ";
		}
	}

	void PrintStarLine(const auto& line)
	{
		for (const auto& word : std::views::reverse(line))
		{
			for (const auto& c : std::views::reverse(word))
			{
				std::cout << c;
			}
			std::cout << "*";
		}
	}

	void PrintReverseWordLine(const auto& line)
	{
		for (const auto& word : std::views::reverse(line))
		{
			for (const auto& c : word)
			{
				std::cout << c;
			}
			std::cout << " ";
		}
	}

	void PrintChangeLine(const auto& line, char cC, char nC)
	{
		for (const auto& word : line)
		{
			for (const auto& c : word)
			{
				if (c == cC)
				{
					std::cout << nC;
				}
				else
					std::cout << c;
			}
			std::cout << " ";
		}
	}

	void PrintNumberLine(const auto& line, char cC, char nC)
	{
		for (const auto& word : line)
		{
			for (const auto& c : word)
			{
				std::cout << c;
				if (std::isdigit(c))
					std::cout << std::endl;
			}
			std::cout << " ";
		}
	}

	void PrintColorWordLine(const auto& line, std::string searchWord)
	{
		for (const auto& word : line)
		{
			if (searchWord == word)
			{
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 9);
				std::cout << word;
				++colorCount;
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			}
			else {
				std::cout << word;
			}
			std::cout << " ";
		}
	}

	void PrintLines(const std::vector<std::vector<std::string>>& lines)
	{
		for (const auto& line : lines)
		{
			PrintLine(line);
			std::cout << std::endl;
		}
	}

	std::optional<std::vector<int>> GetWordCount(const std::vector<std::vector<std::string>>& lines)
	{
		std::vector<int> result;
		for (const auto& line : lines)
		{
			result.push_back(line.size());
		}
		return result;
	}



	void TogleCase(auto& lines)
	{
		for (auto& line : lines)
		{
			for (auto& word : line)
			{
				word = word
					| std::views::transform(ToggleChar)
					| std::ranges::to<std::string>();
			}
		}
	}
}


// Helper

char ToggleChar(char c)
{
	char result = c;
	if (std::isupper(c))
		result = std::tolower(c);
	else if (std::islower(c))
		result = std::toupper(c);
	return result;
}
