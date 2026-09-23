export module hs.renderer;

import std;
export import hs.color;
export import hs.transform;

export namespace hs
{
	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;
		
		virtual void Clear(Color color) = 0;
		virtual void DrawRect(const Transform& transform, Color color) = 0;
	};

	// 컨텍스트 준비 뒤에야 생성 가능 → 완성품 대신 생성 함수
	using RendererFactory = std::function<std::unique_ptr<IRenderer>()>;
}