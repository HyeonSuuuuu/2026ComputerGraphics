export module engine_test.collision;

import std;
import hs.movement;
import hs.collision_system;
import engine_test.expect;

using namespace hs;
using namespace engine_test;

export void RunCollisionTests()
{
	std::cout << "[Confine]\n";
	World world;	// 경계 기본값 -1 ~ 1

	Entity confined = world.Spawn({ .pos = { 0.9f, 0.9f }, .size = { 0.2f, 0.2f } });
	confined.Add<Mover>(Velocity{ .dir = {} }, BoundsResponse::Clamp);
	confined.Add<Confine>(Bounds{ .min{ 0.f, 0.f }, .max{ 0.5f, 0.5f } });

	Entity free = world.Spawn({ .pos = { 0.5f, 0.5f }, .size = { 0.2f, 0.2f } });
	free.Add<Mover>(Velocity{ .dir = {} }, BoundsResponse::Clamp);
	world.Flush();

	CollisionSystem::Tick(world);
	Vec2 pos = confined.GetTransform().pos;
	Expect(std::abs(pos.x - 0.4f) < 1e-5f && std::abs(pos.y - 0.4f) < 1e-5f, "Confine 범위 안으로 밀림 (가장자리가 0.5에 닿음)");
	Expect(free.GetTransform().pos == Vec2{ 0.5f, 0.5f }, "Confine 없으면 월드 경계 그대로 (안에 있으니 안 움직임)");
	Expect(confined.Get<Mover>()->hitBounds && !free.Get<Mover>()->hitBounds, "밀린 것만 hitBounds");

	CollisionSystem::Tick(world);
	Expect(!confined.Get<Mover>()->hitBounds, "벽에 붙어 있기만 하면 다음 Tick엔 false (닿을 때마다 한 번)");
}
