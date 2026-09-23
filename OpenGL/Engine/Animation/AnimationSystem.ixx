export module hs.animation_system;

import std;
import hs.check;
import hs.animation;
export import hs.scene;

export namespace hs
{
	class AnimationSystem
	{
	public:
		static void Tick(Scene& scene, float dt)
		{
			Check(dt >= 0.f, "시간은 거꾸로 흐르지 않는다");

			for (Object& object : scene.Objects())
			{
				Animator* animator = object.Get<Animator>();
				if (!animator || !animator->enabled)
					continue;

				for (auto& [type, animation] : animator->animations)
					if (animation->IsEnabled())
						animation->Update(object, dt);

				std::erase_if(animator->animations, [&object](const auto& entry)
					{
						if (!entry.second->IsFinished())
							return false;

						entry.second->SetEnabled(object, false);	// 제거 전 복원
						return true;
					});
			}
		}
	};
}
