export module hs.render_system;

import hs.renderer;
import hs.scene;

export namespace hs
{
	// 그리는 순서 = 만든 순서
	class RenderSystem
	{
	public:
		static void Draw(const Scene& scene, IRenderer& renderer)
		{
			for (const Object& object : scene.Objects())
				if (object.visual)
					renderer.DrawRect(object.transform, object.visual->color);
		}
	};
}
