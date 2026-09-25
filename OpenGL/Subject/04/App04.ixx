module;
#include <GLFW/glfw3.h>

export module app04;

import std;
import hs;
import app04.rect;

using namespace hs;

export namespace app04
{
	class App04 : public App
	{
		using Super = App;
	public:
		App04(int width, int height)
			: Super(width, height)
		{
			// 맵 범위가 화면과 다를 때만
			// _world.SetBounds({ .min{ -1.f, -1.f }, .max{ 1.f, 1.f } });

			UpdateStatus();
		}

	protected:

		void Update(float dt) override
		{
			const auto& input = GetInput();

			if (input.IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
			{
				if (_world.Size() < MaxSpawnRect)
				{
					Entity entity = Rect::Spawn(_world, input.MousePos());
					Mover* mover = entity.Get<Mover>();
					ApplyMotion(entity);
					SetEffect<ScalePulse>(entity, _pulsing);
					SetEffect<ColorCycle>(entity, _coloring);
					mover->enabled = _moving;
				}
			}

			// 같은 키: 멈춤/재개, 다른 키: 모드 전환
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
					UpdateStatus();
				};

			motionKey(GLFW_KEY_1, Motion::Diagonal);
			motionKey(GLFW_KEY_2, Motion::ZigZag);
			motionKey(GLFW_KEY_3, Motion::EdgePatrol);
			motionKey(GLFW_KEY_M, Motion::Home);		// 처음 생성 위치로
			
			if (input.IsKeyPressed(GLFW_KEY_4))
			{
				_pulsing = !_pulsing;
				for (Entity entity : _world.Entities())
					SetEffect<ScalePulse>(entity, _pulsing);
			}
			
			if (input.IsKeyPressed(GLFW_KEY_5))
			{
				_coloring = !_coloring;
				for (Entity entity : _world.Entities())
					SetEffect<ColorCycle>(entity, _coloring);
			}
			
			if (input.IsKeyPressed(GLFW_KEY_S))
			{
				_moving = false;
				_pulsing = false;
				_coloring = false;

				ApplyEnabled();
				UpdateStatus();
				for (Entity entity : _world.Entities())
				{
					SetEffect<ScalePulse>(entity, false);
					SetEffect<ColorCycle>(entity, false);
				}
			}
			

			
			if (input.IsKeyPressed(GLFW_KEY_R))
				_world.Clear();
			
			if (input.IsKeyPressed(GLFW_KEY_Q))
				Close();

			_world.Flush();						// 목록 확정. 이번 프레임 생성분도 아래 시스템 대상

			EffectSystem::Tick(_world, dt);
			MovementSystem::Tick(_world, dt);
			CollisionSystem::Tick(_world);
		}

		void Render() override
		{
			auto& renderer = GetRenderer();
			renderer.Clear({ 0.15f, 0.15f, 0.18f });
			RenderSystem::Draw(_world, renderer);
		}

	private:
		enum class Motion { Diagonal, ZigZag, EdgePatrol, Home };
		
		void ApplyMotion(Entity entity)
		{
			Mover* mover = entity.Get<Mover>();
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
				if (Home* home = entity.Get<Home>())
					mover->SetMode(std::make_unique<FollowMode>(home->pos));
				mover->boundsResponse = BoundsResponse::Clamp;
				mover->velocity.speed = Rect::Speed;	// 도착 시 FollowMode가 0으로
				break;

			case Motion::EdgePatrol:
				// 정지 상태면 벽 접촉 불가 → 대각선 출발
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
			for (Entity entity : _world.Entities())
				ApplyMotion(entity);
		}
		
		Bounds PatrolArea() const
		{
			Bounds area = _world.WorldBounds();
			const float half = Rect::Size / 2.f;
			area.min += Vec2{ half, half };
			area.max -= Vec2{ half, half };
			return area;
		}

		template<class T>
		void SetEffect(Entity entity, bool on)
		{
			EffectStack* stack = entity.Get<EffectStack>();
			Visual* visual = entity.Get<Visual>();
			if (!stack || !visual)
				return;

			if (T* effect = stack->Get<T>())
				effect->SetEnabled(*visual, on);
		}

		// 방향·모드 유지, 정지만
		void ApplyEnabled()
		{
			for (Entity entity : _world.Entities())
				if (Mover* mover = entity.Get<Mover>())
					mover->enabled = _moving;
		}

		// 모드·정지가 바뀔 때만 부름: 창 제목 변경은 운영체제 호출
		void UpdateStatus()
		{
			SetStatus(Format("{}{}", EnumToString(_motion), _moving ? "" : " (정지)"));
		}


		World							_world;
		Motion							_motion = Motion::Diagonal;
		bool							_moving = false;
		bool							_pulsing = false;
		bool							_coloring = false;
		static constexpr std::uint32_t		MaxSpawnRect = 5;
		static constexpr float			ZigZagInterval = 0.3f;
	};
}
