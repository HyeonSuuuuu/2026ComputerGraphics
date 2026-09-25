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

			scene.Each<EffectStack, Visual>([dt](Object, EffectStack& stack, Visual& visual)
				{
					if (!stack.enabled)
						return;

					for (auto& [type, effect] : stack.effects)
						if (effect->IsEnabled())
							effect->Update(visual, dt);

					std::erase_if(stack.effects, [&visual](const auto& entry)
						{
							if (!entry.second->IsFinished())
								return false;

							entry.second->SetEnabled(visual, false);	// 제거 전 복원
							return true;
						});
				});
		}
	};
}
