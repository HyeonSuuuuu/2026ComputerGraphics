export module hs.renderer;

import std;
export import hs.color;
export import hs.shape;
export import hs.transform;

export namespace hs
{
	enum class DrawMode
	{
		Fill,				// 면
		Line,				// 도형의 둘레만, 도형 색으로
		FillAndWireframe,	// 면 + 삼각형 변을 검게 겹침(사각형이면 대각선이 보임)
	};

	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;

		virtual void Clear(Color color) = 0;
		// rotation: 라디안, 반시계. 그리기만 돌림(충돌 상자는 그대로)
		virtual void Draw(Shape shape, const Transform& transform, Color color, float rotation) = 0;
		void Draw(Shape shape, const Transform& transform, Color color) { Draw(shape, transform, color, 0.f); }
		virtual void SetDrawMode(DrawMode) {}		// FirstRenderer는 늘 Fill
		// 점들을 차례로 직선으로 이음(좌표 그대로, 회전·크기 없음). FirstRenderer는 없음
		virtual void DrawPolyline(std::span<const Vec2>, Color) {}
	};

	// 컨텍스트 준비 뒤에야 생성 가능 → 완성품 대신 생성 함수
	using RendererFactory = std::function<std::unique_ptr<IRenderer>()>;
}
