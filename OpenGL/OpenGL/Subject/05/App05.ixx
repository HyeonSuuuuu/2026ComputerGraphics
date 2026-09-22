module;
#include <gl/glfw3.h>

export module app05;

import std;
import hs;
import app05.rect;

export namespace hs
{
	class App05 : public App
	{
		using Super = App;
	public:
		App05(int width, int height)
			: Super(width, height, "App05")
		{
			const int count = Random(20, 40);
			for (int i = 0; i < count; ++i)
			{
				Object& obj = Rect::Spawn(_scene, RandomVec2({-0.9f, -0.9f}, {0.9f, 0.9f}));
				/*Mover* mover = obj.Get<Mover>();
				// mover->enabled = false;*/
			}
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

			AnimationSystem::Tick(_scene, dt);
			MovementSystem::Tick(_scene, dt);
			CollisionSystem::Tick(_scene);
			
			_hits.clear();
			CollisionSystem::FindHits(_scene, _hits);

			for (auto& [a, b] : _hits)	// 확인용. 부딪힐 때의 규칙은 여기에
			{
				if (Visual* visual = a->Get<Visual>()) visual->color = HitColor;
				if (Visual* visual = b->Get<Visual>()) visual->color = HitColor;
			}
			_scene.Flush();
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
		// 연출:     object.Add<Animator>().Add<ScalePulse>()             // 여러 개 가능
		// 찾기:     _scene.HitTest(point) / _scene.FindOverlap(object) / _scene.Destroy(object)
		// 게임 쪽 컴포넌트: struct Health : IComponent { ... }; 엔진은 안 건드린다
		Scene						_scene;
		std::vector<Hit>			_hits;
		uint32_t					_spawnCount = 0;

		static constexpr Color				Background{ 1.f, 1.f, 1.f };
		static constexpr Color			HitColor{ 1.f, 0.2f, 0.2f };
		static constexpr uint32_t			MaxSpawnRect = 10;
		
	};
}
