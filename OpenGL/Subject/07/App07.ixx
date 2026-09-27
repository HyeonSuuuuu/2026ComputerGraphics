module;
#include <GLFW/glfw3.h>

export module app07;

import std;
import hs;
import app07.movement;

using namespace hs;

export namespace app07
{
	class App07 : public App
	{
		using Super = App;
	public:
		App07(int width, int height)
			: Super(width, height, [] { return std::make_unique<ModernRenderer>(); })
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

			if (input.IsKeyPressed(GLFW_KEY_P))
			{
				SpawnShape(Shape::Rect, SmallSize);
			}
			if (input.IsKeyPressed(GLFW_KEY_E))
			{
				SpawnShape(Shape::Line, {0.006f, Random(0.1f, 0.5f)});
			}
			if (input.IsKeyPressed(GLFW_KEY_T))
			{
				SpawnShape(Shape::Triangle, RandomVec2(0.1f, 0.4f));
			}
			if (input.IsKeyPressed(GLFW_KEY_R))
			{
				SpawnShape(Shape::Rect, RandomVec2(0.1f, 0.4f));
			}
			if (input.IsKeyPressed(GLFW_KEY_F))
			{
				_wireFrame = !_wireFrame;
				GetRenderer().SetDrawMode(_wireFrame ? DrawMode::FillAndWireframe : DrawMode::Fill);
			}
			
			Vec2 allDir{};
			if (input.IsKeyHeld(GLFW_KEY_1)) allDir.y += 1.f;
			if (input.IsKeyHeld(GLFW_KEY_2)) allDir.y -= 1.f;
			if (input.IsKeyHeld(GLFW_KEY_3)) allDir.x -= 1.f;
			if (input.IsKeyHeld(GLFW_KEY_4)) allDir.x += 1.f;
			bool released = input.IsKeyReleased(GLFW_KEY_1) || input.IsKeyReleased(GLFW_KEY_2) || input.IsKeyReleased(GLFW_KEY_3) || input.IsKeyReleased(GLFW_KEY_4);
			if (allDir != Vec2{} || released)
				MoveAll(Normalize(allDir));
			
			if (input.IsKeyHeld(GLFW_KEY_C))
				_world.Clear();

			_world.Flush();						// 목록 확정. 이번 프레임 생성분도 아래 시스템 대상

			EffectSystem::Tick(_world, dt);
			MovementSystem::Tick(_world, dt);
			CollisionSystem::Tick(_world);
			
			if (input.IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
				Select(_world.HitTest(input.MousePos()));
		}

		void Render() override
		{
			auto& renderer = GetRenderer();
			renderer.Clear(Background);
			RenderSystem::Draw(_world, renderer);
		}

	private:
		void SpawnShape(Shape shape, Vec2 size)
		{
			// Size는 Flush된 것만 셈 → 한 프레임에 여러 키를 같이 누르면 몇 개 넘칠 수 있음
			if (_world.Size() >= MaxShapes)
				return;

			Entity entity = _world.Spawn({.pos = RandomVec2(-0.9f, 0.9f), .size = size});
			entity.Add<Visual>(RandomColor()).shape = shape;
			entity.Add<Mover>(Velocity{.dir = {}, .speed = MoveSpeed}, BoundsResponse::Clamp);
		}
		
		void Select(Entity clicked)
		{
			if (!clicked || clicked.Id() == _selected)
				return;

			ClearSelection();
			if (Visual* visual = clicked.Get<Visual>())
				visual->outline = Outline{ OutlineColor, OutlineWidth };
			if (Mover* mover = clicked.Get<Mover>())
				mover->SetMode(std::make_unique<KeyboardMode>(GetInput()));
			_selected = clicked.Id();
		}

		// 전부 선택 표시하고 같은 방향으로. dir이 0이면 선택된 채로 멈춤
		void MoveAll(Vec2 dir)
		{
			ClearSelection();
			
			_world.Each<Mover, Visual>([&](Entity, Mover& mover, Visual& visual)
				{
					if (dir == Vec2{})
						visual.outline = Outline{ OutlineColor, 0};
					else
						visual.outline = Outline{ OutlineColor, OutlineWidth };
					mover.velocity.dir = dir;
				});
		}

		void ClearSelection()
		{
			_world.Each<Mover, Visual>([](Entity, Mover& mover, Visual& visual)
				{
					visual.outline.reset();
					mover.SetMode(nullptr);
					mover.velocity.dir = {};
				});
			_selected = {};
		}
		
		World							_world;
		EntityId						_selected{};
		bool							_wireFrame = false;

		static constexpr Color			Background{ 1.f, 1.f, 1.f };
		static constexpr Vec2			SmallSize = {0.03f, 0.03f};
		static constexpr Color			OutlineColor = {0.f, 0.f, 0.f};
		static constexpr float			OutlineWidth = 0.02f;
		static constexpr float			MoveSpeed = 0.3f;
		static constexpr std::size_t	MaxShapes = 50;
		
	};

	// main에서 과제 번호만 바꾸면 되게: app09::App. 클래스 이름을 App으로 하면 hs::App과 겹침
	using App = App07;
}
