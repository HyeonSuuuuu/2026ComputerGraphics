export module hs.renderer;

import std;
import hs.shape;

export namespace hs
{
	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;
		
		virtual void Clear(Color color) = 0;
		virtual void DrawRect(const Transform& transform, Color color) = 0;
	};

	// 컨텍스트가 준비된 뒤에야 만들 수 있어서, 완성품 대신 만드는 법을 넘긴다
	using RendererFactory = std::function<std::unique_ptr<IRenderer>()>;
}