module;
#include <gl/glfw3.h>

export module app04;

import std;
import hs;
import app04.rect;

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
				if (_scene.Size() < MaxSpawnRect)
				{
					Object& object = Rect::Spawn(_scene, input.MousePos());
					Mover* mover = object.Get<Mover>();
					ApplyMotion(object);
					SetAnimation<ScalePulse>(object, _pulsing);
					SetAnimation<ColorCycle>(object, _coloring);
					mover->enabled = _moving;
				}
			}

			// 같은 키를 다시 누르면 멈춤/재개, 다른 키면 모드 전환
			auto motionKey = [&](int key, Motion motion)
				{
					if (!input.IsKeyPressed(key))
						return;

					if (_motion == motion)
					{
						_moving = !_moving;
						ApplyEnabled();
					}
					else
					{
						_motion = motion;
						_moving = true;
						ApplyMotion();
					}
				};

			motionKey(GLFW_KEY_1, Motion::Diagonal);
			motionKey(GLFW_KEY_2, Motion::ZigZag);
			motionKey(GLFW_KEY_3, Motion::EdgePatrol);
			motionKey(GLFW_KEY_M, Motion::Home);		// 처음 만들어진 자리로
			
			if (input.IsKeyPressed(GLFW_KEY_4))
			{
				_pulsing = !_pulsing;
				for (Object& object : _scene.Objects())
					SetAnimation<ScalePulse>(object, _pulsing);
			}
			
			if (input.IsKeyPressed(GLFW_KEY_5))
			{
				_coloring = !_coloring;
				for (Object& object : _scene.Objects())
					SetAnimation<ColorCycle>(object, _coloring);
			}
			
			// 전부 정지
			if (input.IsKeyPressed(GLFW_KEY_S))
			{
				_moving = false;
				_pulsing = false;
				_coloring = false;

				ApplyEnabled();
				for (Object& object : _scene.Objects())
				{
					SetAnimation<ScalePulse>(object, false);
					SetAnimation<ColorCycle>(object, false);
				}
			}
			

			
			// 전부 지우기
			if (input.IsKeyPressed(GLFW_KEY_R))
				_scene.Clear();
			
			if (input.IsKeyPressed(GLFW_KEY_Q))
				Close();

			AnimationSystem::Tick(_scene, dt);	// 연출
			MovementSystem::Tick(_scene, dt);	// 속도와 적분
			CollisionSystem::Tick(_scene);		// 겹침 해소는 맨 뒤
			_scene.Flush();
		}

		void Render() override
		{
			auto& renderer = GetRenderer();
			renderer.Clear({ 0.15f, 0.15f, 0.18f });
			RenderSystem::Draw(_scene, renderer);
		}

	private:
		enum class Motion { Diagonal, ZigZag, EdgePatrol, Home };
		
		void ApplyMotion(Object& object)
		{
			Mover* mover = object.Get<Mover>();
			if (!mover)
				return;

			switch (_motion)
			{
			case Motion::Diagonal:
				mover->SetMode(nullptr);				// 모드 없음 = 등속 직선
				mover->boundsResponse = BoundsResponse::Reflect;
				mover->velocity = { .dir = Rect::RandomDiagonal(), .speed = Rect::Speed };
				break;

			case Motion::ZigZag:
				mover->SetMode(std::make_unique<ZigZagMode>(ZigZagInterval));
				mover->boundsResponse = BoundsResponse::Reflect;
				mover->velocity = { .dir = { 1.f, 0.f }, .speed = Rect::Speed };
				break;

			case Motion::Home:
				if (Home* home = object.Get<Home>())
					mover->SetMode(std::make_unique<FollowMode>(home->pos));
				mover->boundsResponse = BoundsResponse::Clamp;
				mover->velocity.speed = Rect::Speed;	// 도착하면 FollowMode가 0으로 만듬
				break;

			case Motion::EdgePatrol:
				// 멈춰 있으면 벽에 닿을 일이 없다. 1번처럼 대각선으로 출발
				if (mover->velocity.speed <= 0.f)
					mover->velocity = { .dir = Rect::RandomDiagonal(), .speed = Rect::Speed };

				mover->SetMode(std::make_unique<EdgePatrolMode>(PatrolArea(), Rect::Speed));
				mover->boundsResponse = BoundsResponse::Clamp;
				break;
			}

			mover->enabled = _moving;
		}

		void ApplyMotion()
		{
			for (Object& object : _scene.Objects())
				ApplyMotion(object);
		}
		
		Bounds PatrolArea() const
		{
			Bounds area = _scene.WorldBounds();
			const float half = Rect::Size / 2.f;
			area.min += Vec2{ half, half };
			area.max -= Vec2{ half, half };
			return area;
		}

		template<class T>
		void SetAnimation(Object& object, bool on)
		{
			if (Animator* animator = object.Get<Animator>())
				if (T* animation = animator->Get<T>())
					animation->SetEnabled(object, on);
		}

		// 방향과 모드는 그대로 두고 멈추기만 한다
		void ApplyEnabled()
		{
			for (Object& object : _scene.Objects())
				if (Mover* mover = object.Get<Mover>())
					mover->enabled = _moving;
		}


		Scene							_scene;
		Motion							_motion = Motion::Diagonal;
		bool							_moving = false;
		bool							_pulsing = false;
		bool							_coloring = false;
		static constexpr uint32_t		MaxSpawnRect = 5;
		static constexpr float			ZigZagInterval = 0.3f;
	};
}
