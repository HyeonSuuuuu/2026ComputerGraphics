export module hs.effect_system;

import std;
import hs.check;
import hs.effect;
export import hs.scene;

export namespace hs
{
	class EffectSystem
	{
	public:
		static void Tick(Scene& scene, float dt)
		{
			Check(dt >= 0.f, "시간은 거꾸로 흐르지 않는다");

			for (Object& object : scene.Objects())
			{
				EffectStack* stack = object.Get<EffectStack>();
				Visual* visual = object.Get<Visual>();
				if (!stack || !stack->enabled || !visual)
					continue;

				for (auto& [type, effect] : stack->effects)
					if (effect->IsEnabled())
						effect->Update(*visual, dt);

				std::erase_if(stack->effects, [visual](const auto& entry)
					{
						if (!entry.second->IsFinished())
							return false;

						entry.second->SetEnabled(*visual, false);	// 제거 전 복원
						return true;
					});
			}
		}
	};
}
