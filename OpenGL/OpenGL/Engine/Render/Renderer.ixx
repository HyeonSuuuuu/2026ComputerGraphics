export module hs.renderer;

import hs.shape;

export namespace hs
{
	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;
		
		virtual void Clear(Color color) = 0;
		virtual void DrawRect(const Rectangle& rect) = 0;
	};
}