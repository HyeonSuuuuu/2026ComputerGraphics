
export module Warmup1;

import std;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> dis(1, 9);

export using matrix2x2 = std::array<std::array<int, 2>, 2>;
export using matrix3x3 = std::array<std::array<int, 3>, 3>;
export using matrix4x4 = std::array<std::array<int, 4>, 4>;

constexpr int ColWidth = 2;

constexpr bool IsMultipliable(const auto& A, const auto& B);
constexpr bool IsSameDimension(const auto& A, const auto& B);

export namespace wu1
{
	void RandomMatrix(auto& matrix)
	{
		for (auto& row : matrix)
		{
			for (auto& value : row)
			{
				value = dis(gen);
			}
		}
	}

	void PrintMatrix(const auto& A)
	{
		for (const auto& row : A)
		{
			for (int value : row)
			{
				std::cout << std::setw(ColWidth) << value << " ";
			}
			std::cout << std::endl;
		}
	}

	void PrintMatrix(const auto& A, const auto& B)
	{
		for (int i = 0; i < A.size(); ++i)
		{
			for (int value : A[i])
			{
				std::cout << std::setw(ColWidth) << value << " ";
			}
			std::cout << "  ";
			for (int value : B[i])
			{
				std::cout << std::setw(ColWidth) << value << " ";
			}
			std::cout << std::endl;
		}
	}

	template <class MatrixA, class MatrixB>
	constexpr std::optional<MatrixA> AddMatrix(const MatrixA& A, const MatrixB& B)
	{
		if (!IsSameDimension(A, B))
			return std::nullopt;

		auto result = A;
		for (int i = 0; i < A.size(); ++i)
		{
			for (int j = 0; j < A[i].size(); ++j)
			{
				result[i][j] = A[i][j] + B[i][j];
			}
		}
		return result;
	}

	template <class MatrixA, class MatrixB>
	constexpr std::optional<MatrixA> SubMatrix(const MatrixA& A, const MatrixB& B)
	{
		if (!IsSameDimension(A, B))
			return std::nullopt;

		auto result = A;
		for (int i = 0; i < A.size(); ++i)
		{
			for (int j = 0; j < A[i].size(); ++j)
			{
				result[i][j] = A[i][j] - B[i][j];
			}
		}
		return result;
	}

	template <class MatrixA, class MatrixB>
	constexpr std::optional<MatrixA> MulMatrix(const MatrixA& A, const MatrixB& B)
	{
		if (!IsMultipliable(A, B))
			return std::nullopt;

		auto result = A;
		for (int i = 0; i < A.size(); ++i)
		{
			for (int j = 0; j < B[0].size(); ++j)
			{
				int product = 0;
				for (int k = 0; k < B.size(); ++k)
				{
					product += A[i][k] * B[k][j];
				}
				result[i][j] = product;
			}
		}
		return result;
	}

	// 전치 행렬
	constexpr auto TMatrix(const auto& A)
	{
		auto result = A;
		for (int row = 0; row < A.size(); ++row)
		{
			for (int col = 0; col < A[row].size(); ++col)
			{
				result[col][row] = A[row][col];
			}
		}
		return result;
	}

	
	constexpr auto SubRowMin(const auto& A)
	{
		auto result = A;
		for (auto& row : result)
		{
			const auto min = std::ranges::min(row);
			for (auto& value : row)
			{
				value -= min;
			}
		}
		return result;
	}

	constexpr auto AddColMax(const auto& A)
	{
		auto result = TMatrix(A);
		for (auto& row : result)
		{
			const auto max = std::ranges::max(row);
			for (auto& value : row)
			{
				value += max;
			}
		} 
		return TMatrix(result);
	}

	// 소행렬
	constexpr matrix3x3 Minor(const matrix4x4& A, int skipRow, int skipCol)
	{
		matrix3x3 result{};
		for (int i = 0, row = 0; i < A.size(); ++i)
		{
			if (i == skipRow)
				continue;
			for (int j = 0, col = 0; j < A[i].size(); ++j)
			{
				if (j == skipCol)
					continue;
				result[row][col] = A[i][j];
				++col;
			}
			++row;
		}
		return result;
	}

	constexpr matrix2x2 Minor(const matrix3x3& A, int skipRow, int skipCol)
	{
		matrix2x2 result{};
		for (int i = 0, r = 0; i < A.size(); ++i)
		{
			if (i == skipRow)
				continue;
			for (int j = 0, c = 0; j < A[i].size(); ++j)
			{
				if (j == skipCol)
					continue;
				result[r][c] = A[i][j];
				++c;
			}
			++r;
		}
		return result;
	}

	constexpr int DetMatrix(const matrix2x2& A)
	{
		return A[0][0] * A[1][1] - A[0][1] * A[1][0];
	}

	constexpr int DetMatrix(const matrix3x3& A)
	{
		int det = 0;
		int sign = 1;
		for (int col = 0; col < A[0].size(); ++col)
		{
			det += sign * A[0][col] * DetMatrix(Minor(A, 0, col));
			sign = -sign;
		}
		return det;
	}

	constexpr int DetMatrix(const matrix4x4& A)
	{
		int det = 0;
		int sign = 1;
		for (int col = 0; col < A[0].size(); ++col)
		{
			det += sign * A[0][col] * DetMatrix(Minor(A, 0, col));
			sign = -sign;
		}
		return det;
	}
	
	constexpr auto AddOneMatrix(const auto& A)
	{
		auto result = A;
		for (auto& row : result)
		{
			for (auto& value : row)
			{
				if (value < 9)
					value += 1;
			}
		}
		return result;
	}

	constexpr auto SubOneMatrix(const auto& A)
	{
		auto result = A;
		for (auto& row : result)
		{
			for (auto& value : row)
			{
				if (value > 0)
					value -= 1;
			}
		}
		return result;
	}
}

// =========== Helper =================
constexpr bool IsSameDimension(const auto& A, const auto& B)
{
	if (A.size() != B.size())
		return false;
	for (int i = 0; i < A.size(); ++i)
	{
		if (A[i].size() != B[i].size())
			return false;
	}
	return true;
}

constexpr bool IsMultipliable(const auto& A, const auto& B)
{
	return !A.empty() && A[0].size() == B.size();
}

