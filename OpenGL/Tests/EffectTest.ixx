export module engine_test.effect;

import std;
import hs.effects;
import hs.effect_system;
import engine_test.expect;

using namespace hs;
using namespace engine_test;

export void RunEffectTests()
{
	std::cout << "[Effect]\n";
	World world;
	Entity entity = world.Spawn({ .pos = {}, .size = { 1.f, 1.f } });
	entity.Add<Visual>(Color{ 1.f, 0.f, 0.f });
	entity.Add<EffectStack>().Add<ColorFade>(Color{ 1.f, 0.f, 0.f }, Color{ 0.f, 0.f, 0.f }, 1.f);
	world.Flush();

	EffectSystem::Tick(world, 0.5f);
	Expect(std::abs(entity.Get<Visual>()->color.r - 0.5f) < 1e-4f, "ColorFade: 절반 지점");

	EffectSystem::Tick(world, 1.f);
	Expect(entity.Get<EffectStack>()->effects.empty(), "끝나면 목록에서 빠짐");
	Expect(entity.Get<Visual>()->color.r == 0.f, "끝난 뒤에도 끝 색 유지 (시작 색으로 튀지 않음)");
}
