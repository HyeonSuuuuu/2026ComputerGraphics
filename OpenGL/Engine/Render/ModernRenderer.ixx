module;
#include <GL/glew.h>

export module hs.modern_renderer;

import std;
import hs.enums;
import hs.shader;
export import hs.renderer;

namespace hs
{
	// 컴파일할 때 박아 넣음 → 실행 폴더와 무관. 끝의 0은 문자열 끝 표시
	// 셰이더 파일은 ASCII만
	constexpr char SolidVertexSource[] = {
#embed "Shaders/Solid.vert"
		, 0 };
	constexpr char SolidFragmentSource[] = {
#embed "Shaders/Solid.frag"
		, 0 };
}

export namespace hs
{
	// 셰이더로 그리는 렌더러. 모든 단위 도형을 VBO(점)·EBO(삼각형 번호) 하나씩에 이어 올려 두고,
	// 그릴 때는 uniform(위치·크기·색)만 바꾸고 그 도형의 구간만 그림
	// 면: EBO로 GL_TRIANGLES   선 모양: 점 순서대로 GL_LINES   DrawMode::Line: 점 순서대로 GL_LINE_LOOP
	class ModernRenderer final : public IRenderer
	{
	public:
		ModernRenderer()
			: _solid(SolidVertexSource, SolidFragmentSource)
			, _posLocation(_solid.UniformLocation("uPos"))
			, _sizeLocation(_solid.UniformLocation("uSize"))
			, _colorLocation(_solid.UniformLocation("uColor"))
			, _rotationLocation(_solid.UniformLocation("uRotation"))
			, _aspectLocation(_solid.UniformLocation("uAspect"))
		{
			std::vector<Vec2> points;
			std::vector<std::uint32_t> indices;
			for (std::size_t i = 0; i < EnumCount<Shape>; ++i) {
				Shape shape = static_cast<Shape>(i);
				auto polygon = UnitPolygon(shape);
				auto triangles = UnitTriangles(shape);

				_ranges[i] = {
					.first = static_cast<GLint>(points.size()),
					.count = static_cast<GLsizei>(polygon.size()),
					.indexFirst = indices.size(),
					.indexCount = static_cast<GLsizei>(triangles.size()),
				};
				// 도형마다 0번부터 센 번호 → 이 VBO 안의 실제 위치로 밀어 줌
				for (std::uint32_t index : triangles)
					indices.push_back(static_cast<std::uint32_t>(points.size()) + index);
				points.append_range(polygon);
			}

			glGenVertexArrays(1, &_vao);
			glGenBuffers(1, &_vbo);
			glGenBuffers(1, &_ebo);

			glBindVertexArray(_vao);
			glBindBuffer(GL_ARRAY_BUFFER, _vbo);
			glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vec2), points.data(), GL_STATIC_DRAW);
			// EBO 바인딩은 VAO에 저장됨 → VAO가 묶인 동안
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(std::uint32_t), indices.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), nullptr);
			glEnableVertexAttribArray(0);

			glBindVertexArray(0);

			// 경로 선: 점이 매번 달라서 도형과 따로. 그릴 때마다 새로 올림
			glGenVertexArrays(1, &_lineVao);
			glGenBuffers(1, &_lineVbo);
			glBindVertexArray(_lineVao);
			glBindBuffer(GL_ARRAY_BUFFER, _lineVbo);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), nullptr);
			glEnableVertexAttribArray(0);
			glBindVertexArray(0);
		}

		~ModernRenderer() override
		{
			glDeleteVertexArrays(1, &_vao);
			glDeleteBuffers(1, &_vbo);
			glDeleteBuffers(1, &_ebo);
			glDeleteVertexArrays(1, &_lineVao);
			glDeleteBuffers(1, &_lineVbo);
		}

		void Clear(Color color) override
		{
			glClearColor(color.r, color.g, color.b, 1.f);
			glClear(GL_COLOR_BUFFER_BIT);

			// 회전은 화면 비율 공간에서 해야 모양이 안 찌그러짐(NDC는 가로·세로 픽셀 길이가 다름)
			// 매 프레임 한 번: 창 크기가 바뀌어도 맞음
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);
			_aspect = viewport[3] > 0 ? static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]) : 1.f;
		}

		void Draw(Shape shape, const Transform& transform, Color color, float rotation) override
		{
			const Range& range = Prepare(shape, transform, color, rotation);
			if (IsLine(shape)) {
				glDrawArrays(GL_LINES, range.first, range.count);
				return;
			}

			switch (_drawMode)
			{
			case DrawMode::Fill:
				DrawTriangles(range);
				break;
			case DrawMode::Line:
				// 점이 둘레 순서라 그대로 이으면 둘레. 사각형도 대각선 없이
				glDrawArrays(GL_LINE_LOOP, range.first, range.count);
				break;
			case DrawMode::FillAndWireframe:
				DrawTriangles(range);
				// 같은 삼각형을 변만 한 번 더. 채우기 모드는 전역 상태라 바로 되돌림
				Shader::Set(_colorLocation, WireframeColor);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				DrawTriangles(range);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				break;
			}
		}

		void SetDrawMode(DrawMode mode) override { _drawMode = mode; }

		void DrawPolyline(std::span<const Vec2> points, Color color) override
		{
			if (points.size() < 2)
				return;

			_solid.Use();
			// 크기 1·위치 0·회전 0 → 셰이더가 좌표를 그대로 통과
			Shader::Set(_posLocation, Vec2{});
			Shader::Set(_sizeLocation, Vec2{ 1.f, 1.f });
			Shader::Set(_colorLocation, color);
			Shader::Set(_rotationLocation, 0.f);
			Shader::Set(_aspectLocation, _aspect);

			glBindVertexArray(_lineVao);
			glBindBuffer(GL_ARRAY_BUFFER, _lineVbo);
			glBufferData(GL_ARRAY_BUFFER, points.size_bytes(), points.data(), GL_DYNAMIC_DRAW);
			glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(points.size()));
		}

	private:
		// first·count: VBO의 점 구간   indexFirst·indexCount: EBO의 삼각형 번호 구간
		struct Range { GLint first; GLsizei count; std::size_t indexFirst; GLsizei indexCount; };

		const Range& Prepare(Shape shape, const Transform& transform, Color color, float rotation)
		{
			// 앱의 Render가 사이에 다른 프로그램·VAO를 묶었을 수 있어 매번 묶음
			_solid.Use();
			glBindVertexArray(_vao);

			Shader::Set(_posLocation, transform.pos);
			Shader::Set(_sizeLocation, transform.size);
			Shader::Set(_colorLocation, color);
			Shader::Set(_rotationLocation, rotation);
			Shader::Set(_aspectLocation, _aspect);
			return _ranges[static_cast<std::size_t>(shape)];
		}

		static void DrawTriangles(const Range& range)
		{
			glDrawElements(GL_TRIANGLES, range.indexCount, GL_UNSIGNED_INT,
				reinterpret_cast<const void*>(range.indexFirst * sizeof(std::uint32_t)));
		}

		Shader _solid;
		GLint _posLocation, _sizeLocation, _colorLocation, _rotationLocation, _aspectLocation;
		GLuint _vao{}, _vbo{}, _ebo{};
		GLuint _lineVao{}, _lineVbo{};
		std::array<Range, EnumCount<Shape>> _ranges{};
		DrawMode _drawMode = DrawMode::Fill;
		float _aspect = 1.f;	// 가로 / 세로. Clear에서 갱신

		static constexpr Color WireframeColor{ 0.f, 0.f, 0.f };
	};
}
