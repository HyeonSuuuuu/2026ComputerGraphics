module;
#include <gl/glm/glm.hpp>

export module hs.render_system;

import hs.shape;
import hs.renderer;
import hs.visual;
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
				if (const Visual* visual = object.IsAlive() ? object.Get<Visual>() : nullptr)
				{
					Transform drawn = object.transform;
					drawn.size *= visual->scale;		// 연출용 배율은 그릴 때만 적용한다
					renderer.DrawRect(drawn, visual->color);
				}
		}
	};
}
