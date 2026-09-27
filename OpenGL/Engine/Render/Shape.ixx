export module hs.shape;

import std;
export import hs.vec2;

namespace hs
{
	constexpr Vec2 RectPolygon[] = { { -0.5f, -0.5f }, { 0.5f, -0.5f }, { 0.5f, 0.5f }, { -0.5f, 0.5f } };
	constexpr Vec2 RightTrianglePolygon[] = { { -0.5f, -0.5f }, { 0.5f, -0.5f }, { -0.5f, 0.5f } };
	constexpr Vec2 TrianglePolygon[] = { { -0.5f, -0.5f }, { 0.5f, -0.5f }, { 0.f, 0.5f } };
	constexpr Vec2 LinePolygon[] = {{-0.f, -0.5f}, {0.f, 0.5f}};

	constexpr std::uint32_t RectTriangles[] = { 0, 1, 2,   0, 2, 3 };
	constexpr std::uint32_t OneTriangle[] = { 0, 1, 2 };
}

export namespace hs
{
	// 정삼각형은 Triangle에 크기 {w, w * √3/2}
	enum class Shape { Rect, RightTriangle, Triangle, Line };

	// 한 변이 1인 상자(-0.5 ~ 0.5)를 채우는 꼭짓점. 둘레를 도는 순서
	std::span<const Vec2> UnitPolygon(Shape shape)
	{
		switch (shape)
		{
		case Shape::Rect:          return RectPolygon;
		case Shape::RightTriangle: return RightTrianglePolygon;
		case Shape::Triangle:      return TrianglePolygon;
		case Shape::Line:		   return LinePolygon;
		}
		std::unreachable();
	}
	
	// 면을 이루는 삼각형. UnitPolygon의 번호 세 개씩. 선은 면이 없어 비어 있음
	std::span<const std::uint32_t> UnitTriangles(Shape shape)
	{
		switch (shape)
		{
		case Shape::Rect:          return RectTriangles;
		case Shape::RightTriangle: return OneTriangle;
		case Shape::Triangle:      return OneTriangle;
		case Shape::Line:          return {};
		}
		std::unreachable();
	}

	bool IsLine(Shape shape) { return shape == Shape::Line; }
}
