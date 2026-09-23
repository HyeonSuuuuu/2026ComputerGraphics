module;
#include <GLFW/glfw3.h>

export module app05;

import std;
import hs;
import app05.rect;

using namespace hs;

export namespace app05
{
	struct Spawned : IComponent { };

	// 먹을수록 커지는 목표 크기. 실제 크기는 Grow가 따라감
	struct Growth : IComponent
	{
		explicit Growth(Vec2 target) : target(target) {}

		Vec2 target;
	};

	
	class App05 : public App
	{
		using Super = App;
	public:
		App05(int width, int height)
			: Super(width, height, "App05")
		{
			Restart();

			// 맵 범위가 화면과 다를 때만
			// _scene.SetBounds({ .min{ -1.f, -1.f }, .max{ 1.f, 1.f } });
		}

	protected:

		void Update(float dt) override
		{
			const auto& input = GetInput();

			if (input.IsKeyPressed(GLFW_KEY_Q))
				Close();

			if (input.IsKeyPressed(GLFW_KEY_R))
				Restart();

			UpdateEraser(input);

			if (input.IsMouseReleased(GLFW_MOUSE_BUTTON_RIGHT))
			{
				if (_createCount < MaxCreateRect)
				{
					Object& spawned = Rect::Spawn(_scene, input.MousePos());
					spawned.Add<Spawned>();
					_contacts.AssumeTouching(spawned.Id(), _eraser);
					_createCount++;
				}
			}

			_scene.Flush(); // 목록 확정. 이번 프레임 생성분도 아래 시스템 대상

			Grow(dt);
			EffectSystem::Tick(_scene, dt);
			MovementSystem::Tick(_scene, dt);
			CollisionSystem::Tick(_scene);
			CollisionSystem::Separate(_scene);
			
			for (const Hit& hit : _contacts.Update(_scene))
			{
				if (hit.phase != HitPhase::Enter)
					continue;

				Object* a = _scene.Find(hit.a);
				Object* b = _scene.Find(hit.b);
				if (!a || !b)
					continue;
				
				Object* eraser = a->Get<Trigger>() ? a : (b->Get<Trigger>() ? b : nullptr);
				if (!eraser)
					continue;

				Eat(*eraser, eraser == a ? *b : *a);
			}
		}

		void Render() override
		{
			auto& renderer = GetRenderer();
			renderer.Clear(Background);
			RenderSystem::Draw(_scene, renderer);
		}

	private:
		// 남은 ObjectId는 Clear의 세대 증가로 전부 무효
		void Restart()
		{
			_scene.Clear();
			_contacts.Clear();
			_eraser = {};
			_createCount = 0;

			const int count = Random(20, 40);
			for (int i = 0; i < count; ++i)
				Rect::Spawn(_scene, RandomVec2({ -0.9f, -0.9f }, { 0.9f, 0.9f }));
		}

		// 면적 합산 (과제3 합치기와 동일)
		// 커질수록 한 개의 비중 감소 → 성장 자연 감속
		static Vec2 Merged(Vec2 size, Vec2 eaten)
		{
			const float area = size.x * size.y + eaten.x * eaten.y;
			const float aspect = (size.x + eaten.x) / (size.y + eaten.y);

			const float width = std::sqrt(area * aspect);
			return { width, area / width };
		}

		// transform.size를 키움 → 판정 범위도 함께 확대
		void Grow(float dt)
		{
			for (Object& object : _scene.Objects())
				if (Growth* growth = object.Get<Growth>())
					object.transform.size = Approach(object.transform.size, growth->target, GrowRate, dt);
		}

		void Eat(Object& eraser, Object& target)
		{
			if (Visual* eaten = target.Get<Visual>())
				if (Visual* visual = eraser.Get<Visual>())
					visual->color = eaten->color;

			if (Growth* growth = eraser.Get<Growth>())
				growth->target = Merged(growth->target, target.transform.size);
			
			if (target.Get<Spawned>() != nullptr)
				_createCount--;
			
			_scene.Destroy(target);
		}

		void UpdateEraser(const Input& input)
		{
			if (input.IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
			{
				Object& eraser = _scene.Spawn({ .pos = input.MousePos(), .size = { EraserSize, EraserSize } });
				eraser.Add<Visual>(Color{ 0.f, 0.f, 0.f });
				eraser.Add<Trigger>();
				eraser.Add<EffectStack>().Add<ScaleIn>(0.f, 0.3f);
				eraser.Add<Growth>(Vec2{ EraserSize, EraserSize });
				_eraser = eraser.Id();
				return;						// 목록 반영은 아래 Flush
			}

			Object* eraser = _scene.Find(_eraser);
			if (!eraser)
				return;

			eraser->transform.pos = input.MousePos();

			if (input.IsMouseReleased(GLFW_MOUSE_BUTTON_LEFT))
			{
				_scene.Destroy(*eraser);
				_eraser = {};
			}
		}


		Scene						_scene;
		ContactTracker				_contacts;
		ObjectId					_eraser;	// 안 누르면 빈 값
		std::uint32_t					_createCount = 0;

		static constexpr Color				Background{ 1.f, 1.f, 1.f };
		static constexpr float			Gap = 0.02f;			// 사각형 사이 최소 간격
		static constexpr float			EraserSize = Rect::Size * 2.f;
		static constexpr float			GrowRate = 10.f;		// 초당 남은 차이의 비율
		static constexpr Color			HitColor{ 1.f, 0.2f, 0.2f };
		static constexpr Color			RestColor{ 0.3f, 0.6f, 1.f };
		static constexpr int			MaxCreateRect = 10;
		
		
	};
}
