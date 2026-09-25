// 테스트용 컴포넌트. main과 다른 모듈에서 Add해야 "모듈을 넘어도 같은 저장소" 검사가 됨
export module engine_test.components;

import std;
import hs.world;

export namespace engine_test
{
	struct Home { hs::Vec2 pos; };
	struct Speed { float value; };
	struct Tag { };
	struct Owned { std::unique_ptr<int> value; };	// 이동만 되는 컴포넌트

	hs::Entity SpawnWithHome(hs::World& world, float x)
	{
		hs::Entity entity = world.Spawn({ .pos = { x, 0.f }, .size = { 0.1f, 0.1f } });
		entity.Add<Home>(hs::Vec2{ x, 1.f });
		return entity;
	}
}
