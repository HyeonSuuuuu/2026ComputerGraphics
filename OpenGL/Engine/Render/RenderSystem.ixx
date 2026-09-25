export module hs.render_system;

import hs.transform;
export import hs.renderer;
import hs.visual;
export import hs.world;

export namespace hs
{
	// 그리는 순서 = 칸 순서(Objects). Each는 삭제 후 순서가 섞여서 안 씀
	class RenderSystem
	{
	public:
		static void Draw(World& world, IRenderer& renderer)
		{
			for (Entity entity : world.Entities())
				if (const Visual* visual = entity.Get<Visual>())
				{
					Transform drawn = entity.GetTransform();
					drawn.size *= visual->scale * visual->effectScale;		// 연출 배율은 그릴 때만
					renderer.DrawRect(drawn, visual->color);
				}
		}
	};
}
