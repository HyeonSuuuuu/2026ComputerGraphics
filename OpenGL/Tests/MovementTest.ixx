export module engine_test.movement;

import std;
import hs.movement_modes;
import hs.movement_system;
import hs.collision_system;
import engine_test.expect;

using namespace hs;
using namespace engine_test;

export void RunMovementTests()
{
	std::cout << "[Sweep]\n";
	{
		World world;
		world.SetBounds({ .min{ 0.f, 0.f }, .max{ 1.f, 1.f } });
		Entity entity = world.Spawn({ .pos = { 0.5f, 0.9f }, .size = { 0.1f, 0.1f } });
		entity.Add<Mover>(Velocity{ .dir = { 1.f, 0.f }, .speed = 1.f }, BoundsResponse::Reflect)
			.SetMode(std::make_unique<SweepMode>(0.2f));
		world.Flush();

		auto tick = [&] { MovementSystem::Tick(world, 1.f / 60.f); CollisionSystem::Tick(world); };
		for (int i = 0; i < 30; ++i) tick();		// 0.5초: 오른쪽 벽(0.95)에 닿고 내려가기 시작
		const float yAfterWall = entity.GetTransform().pos.y;
		for (int i = 0; i < 20; ++i) tick();		// 한 칸(0.2) 내려가는 시간 약 0.2초 + 여유
		Expect(std::abs(yAfterWall - 0.9f) < 0.05f && std::abs(entity.GetTransform().pos.y - 0.7f) < 0.02f, "옆 벽에 닿으면 한 칸 내려감");
		Expect(entity.Get<Mover>()->velocity.dir.x < 0.f, "내려간 뒤 반대 방향으로 가로 이동");

		for (int i = 0; i < 600; ++i) tick();		// 10초: 바닥에 닿고 다시 올라가기 시작
		float lowest = 1.f, risen = 0.f;
		for (int i = 0; i < 600; ++i) {
			tick();
			float y = entity.GetTransform().pos.y;
			if (y < lowest) { lowest = y; risen = 0.f; }
			risen = std::max(risen, y - lowest);
		}
		Expect(lowest >= 0.05f - 1e-4f, "바닥 밖으로 안 나감");
		Expect(risen > 0.15f, "바닥에 닿은 뒤엔 위로 올라감");
	}

	std::cout << "[Path]\n";
	{
		World world;
		const std::vector<Vec2> points{ { 0.f, 0.f }, { 0.5f, 0.f }, { 0.5f, 0.5f } };	// 길이 1
		Entity entity = world.Spawn({ .pos = points.front(), .size = { 0.1f, 0.1f } });
		entity.Add<Mover>(Velocity{}, BoundsResponse::None)
			.SetMode(std::make_unique<PathMode>(points, 1.f));
		world.Flush();

		auto tickFor = [&](float seconds) {
			for (int i = 0; i < static_cast<int>(seconds * 100.f + 0.5f); ++i)
				MovementSystem::Tick(world, 0.01f);
		};
		tickFor(0.75f);
		Vec2 pos = entity.GetTransform().pos;
		Expect(Length(pos - Vec2{ 0.5f, 0.25f }) < 1e-3f, "꺾인 점을 지나 두 번째 선 위 (거리 0.75)");
		tickFor(0.5f);
		pos = entity.GetTransform().pos;
		Expect(Length(pos - Vec2{ 0.5f, 0.25f }) < 1e-3f, "끝(거리 1)에서 되돌아옴: 0.25 더 갔다가 0.25 돌아와 같은 자리");
		tickFor(0.75f);
		Expect(Length(entity.GetTransform().pos - points.front()) < 1e-3f, "처음 점으로 돌아옴");
	}
}
