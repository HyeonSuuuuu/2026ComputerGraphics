module;
#include <gl/glfw3.h>

export module app_name;

import std;
import hs;

export namespace hs
{
	class App_name : public App
	{
		using Super = App;
	public:
		App_name(int width, int height)
			: Super(width, height, "app_name")
		{
			// 맵 범위가 화면과 다르면 여기서 한 번만 설정한다
			// _scene.SetBounds({ .min{ -1.f, -1.f }, .max{ 1.f, 1.f } });
		}

	protected:

		void Update(float dt) override
		{
			const auto& input = GetInput();

			if (input.IsKeyPressed(GLFW_KEY_Q))
				Close();

			// 이 과제의 입력과 규칙은 여기에

			MovementSystem::Tick(_scene, dt);
		}

		void Render() override
		{
			auto& renderer = GetRenderer();
			renderer.Clear(Background);
			RenderSystem::Draw(_scene, renderer);
		}

	private:
		// 만들기: _scene.Spawn({ .pos = pos, .size = { 0.12f, 0.12f } })
		// 붙이기: object.visual.Add(RandomColor()) / object.mover.Add(Velocity{ .dir = d, .speed = 0.5f })
		// 찾기:   _scene.HitTest(point) / _scene.FindOverlap(object) / _scene.Destroy(&object)
		Scene						_scene;

		static constexpr Color		Background{ 0.15f, 0.15f, 0.18f };
	};
}
