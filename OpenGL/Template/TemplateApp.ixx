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
			// _scene.SetBounds({ .min{ -1.f, -1.f }, .max{ 1.f, 1.f } });
		}

	protected:

		void Update(float dt) override
		{
			const auto& input = GetInput();

			if (input.IsKeyPressed(GLFW_KEY_Q))
				Close();

			// 과제별 입력과 규칙

			_scene.Flush();						// 목록 확정. 이번 프레임 생성분도 아래 시스템 대상

			EffectSystem::Tick(_scene, dt);
			MovementSystem::Tick(_scene, dt);
			CollisionSystem::Tick(_scene);
		}

		void Render() override
		{
			auto& renderer = GetRenderer();
			renderer.Clear(Background);
			RenderSystem::Draw(_scene, renderer);
		}

	private:
		// 만들기:   Object& object = _scene.Spawn({ .pos = pos, .size = { 0.12f, 0.12f } })
		// 붙이기:   object.Add<Visual>(RandomColor()) / object.Add<Mover>(Velocity{ .dir = d, .speed = 0.5f })
		// 이동방식: mover->SetMode(std::make_unique<ZigZagMode>(0.3f))   // 한 번에 하나
		// 연출:     object.Add<EffectStack>().Add<ScalePulse>()          // 여러 개 가능, Visual만 바꿈
		// 찾기:     _scene.HitTest(point) / _scene.FindOverlap(object) / _scene.Destroy(object)
		// 게임 쪽 컴포넌트: struct Health : IComponent { ... }; 엔진 수정 불필요
		Scene						_scene;

		static constexpr Color		Background{ 0.15f, 0.15f, 0.18f };
	};
}
