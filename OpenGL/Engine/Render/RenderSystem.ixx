export module hs.render_system;

import hs.transform;
export import hs.renderer;
import hs.visual;
export import hs.scene;

export namespace hs
{
	// 그리는 순서 = 생성 순서
	class RenderSystem
	{
	public:
		static void Draw(const Scene& scene, IRenderer& renderer)
		{
			for (const Object& object : scene.Objects())
				if (const Visual* visual = object.Get<Visual>())
				{
					Transform drawn = object.transform;
					drawn.size *= visual->scale * visual->effectScale;		// 연출 배율은 그릴 때만
					renderer.DrawRect(drawn, visual->color);
				}
		}
	};
}
