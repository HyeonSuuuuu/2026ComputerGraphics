module;
#include <GLFW/glfw3.h>

export module app_name;

import std;
import hs;

using namespace hs;

export namespace app_name
{
	class App_name : public App
	{
		using Super = App;
	public:
		App_name(int width, int height)
			: Super(width, height, "app_name")
		{
			// 맵 범위가 화면과 다를 때만
			// _world.SetBounds({ .min{ -1.f, -1.f }, .max{ 1.f, 1.f } });
		}

	protected:

		void Update(float dt) override
		{
			const auto& input = GetInput();

			if (input.IsKeyPressed(GLFW_KEY_Q))
				Close();

			// 과제별 입력과 규칙

			_world.Flush();						// 목록 확정. 이번 프레임 생성분도 아래 시스템 대상

			EffectSystem::Tick(_world, dt);
			MovementSystem::Tick(_world, dt);
			CollisionSystem::Tick(_world);
		}

		void Render() override
		{
			auto& renderer = GetRenderer();
			renderer.Clear(Background);
			RenderSystem::Draw(_world, renderer);
		}

	private:
		// 만들기:   Entity entity = _world.Spawn({ .pos = pos, .size = { 0.12f, 0.12f } })
		// 붙이기:   entity.Add<Visual>(RandomColor()) / entity.Add<Mover>(Velocity{ .dir = d, .speed = 0.5f })
		// 이동방식: mover->SetMode(std::make_unique<ZigZagMode>(0.3f))   // 한 번에 하나
		// 연출:     entity.Add<EffectStack>().Add<ScalePulse>()          // 여러 개 가능, Visual만 바꿈
		// 돌기:     _world.Each<Mover, Transform>([](Entity, Mover& m, Transform& t) { ... })   // 안에서 Add 금지
		// 찾기:     _world.HitTest(point) / _world.FindOverlap(entity) / _world.Destroy(entity)
		// 게임 쪽 컴포넌트: struct Health { ... }; 엔진 수정 불필요
		World						_world;

		static constexpr Color		Background{ 0.15f, 0.15f, 0.18f };
	};
}
