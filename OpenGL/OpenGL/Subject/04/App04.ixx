module;
#include <gl/glfw3.h>

export module app04;

import std;
import hs;

export namespace hs
{
	class App04 : public App
	{
		using Super = App;
	public:
		App04(int width, int height)
			: Super(width, height, "App04")
		{
			// 맵 범위가 화면과 다르면 여기서 한 번만 설정한다
			// _scene.SetBounds({ .min{ -1.f, -1.f }, .max{ 1.f, 1.f } });
		}

	protected:

		void Update(float dt) override
		{
			const auto& input = GetInput();

			if (input.IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
			{
				if (_scene.Size() >= MaxSpawnRect )
					return;
				Spawn(input.MousePos());
			}
			if (input.IsKeyHeld(GLFW_KEY_1))
			{
				
			}
			if (input.IsKeyPressed(GLFW_KEY_Q))
				Close();

			MovementSystem::Tick(_scene, dt);
		}

		void Render() override
		{
			auto& renderer = GetRenderer();
			renderer.Clear({ 0.15f, 0.15f, 0.18f });
			RenderSystem::Draw(_scene, renderer);
		}

	private:
		void Spawn(Vec2 pos)
		{
			Object& object = _scene.Spawn({ .pos = pos, .size = { RectSize, RectSize } });
			object.visual.Add(RandomColor());

			// 움직일 필요가 없으면 이 줄을 빼면 된다
			//Mover& mover = object.mover.Add(Velocity{ .dir = Normalize(RandomVec2({ -1.f, -1.f }, { 1.f, 1.f })), .speed = 0.5f });

			// 이동 방식은 하나만 유효하다. 교체하면 이전 것은 사라진다
			// mover.SetMode(std::make_unique<ZigZagMode>(0.3f));

			// 잠깐 얹히는 이동. 끝나면 원래 움직임으로 돌아간다
			// mover.AddLayeredMove(std::make_unique<MoveTo>(target, 0.4f, EaseOut));
		}

		Scene							_scene;

		static constexpr float			RectSize = 0.12f;
		
		static constexpr uint32_t		MaxSpawnRect = 5;
	};
}
