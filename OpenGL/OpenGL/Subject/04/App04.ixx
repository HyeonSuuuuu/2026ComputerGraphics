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
		{}

	protected:

		void Update(float dt) override
		{
			const auto& input = GetInput();

			if (input.IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
				Spawn(input.MousePos());

			if (input.IsKeyPressed(GLFW_KEY_1))		SetModeAll(nullptr);						// 등속 직선
			if (input.IsKeyPressed(GLFW_KEY_2))		SetModeAll([] { return std::make_unique<ZigZagMode>(ZigZagInterval); });
			if (input.IsKeyPressed(GLFW_KEY_3))		SetModeAll([&] { return std::make_unique<FollowMode>(input.MousePos()); });

			if (input.IsKeyPressed(GLFW_KEY_R))		ReturnAll();							// 원위치 복귀
			if (input.IsKeyPressed(GLFW_KEY_K))		KnockbackAll();							// 넉백
			if (input.IsKeyPressed(GLFW_KEY_SPACE))	_paused = !_paused;
			if (input.IsKeyPressed(GLFW_KEY_C))		Reset();

			if (input.IsKeyPressed(GLFW_KEY_EQUAL))	ScaleSpeed(1.25f);
			if (input.IsKeyPressed(GLFW_KEY_MINUS))	ScaleSpeed(0.8f);

			if (input.IsKeyPressed(GLFW_KEY_Q))		Close();

			// 추적 모드는 매 프레임 목표를 갱신한다
			_registry.Movement().ForEach([&](MovementSystem::Mover& mover) {
				if (auto* follow = dynamic_cast<FollowMode*>(mover.mode.get()))
					follow->SetTarget(input.MousePos());
				});

			if (_paused)
				return;

			// dt 상한을 두지 않으면 창을 드래그하는 동안 경계를 크게 넘어간다
			_registry.Movement().Tick(_registry.Rects(), std::min(dt, MaxDelta));
		}

		void Render() override
		{
			auto& renderer = GetRenderer();
			renderer.Clear({ 0.15f, 0.15f, 0.18f });
			for (const auto& entry : _registry.Rects().Entries())
				renderer.DrawRect(entry.rect);
		}

	private:
		void Spawn(Vec2 pos)
		{
			if (_registry.Rects().Size() >= MaxRectCount)
				return;

			EntityId id = _registry.Spawn({ .pos = pos, .size = { RectSize, RectSize }, .color = RandomColor() });
			_registry.Movement().Attach(id,
				{ .dir = Normalize(RandomVec2({ -1.f, -1.f }, { 1.f, 1.f })), .speed = Random(0.3f, 0.7f) });

			_origins[id] = pos;
		}

		void Reset()
		{
			_registry.Clear();
			_origins.clear();
		}

		// 이동 방식은 하나만 유효하므로 교체한다. make가 없으면 등속 직선으로 돌아간다
		void SetModeAll(auto make)
		{
			for (const auto& entry : _registry.Rects().Entries())
			{
				if constexpr (std::is_null_pointer_v<decltype(make)>)
					_registry.Movement().SetMode(entry.id, nullptr);
				else
					_registry.Movement().SetMode(entry.id, make());
			}
		}

		// 원위치로 부드럽게 복귀. 도착하면 원래 속도로 다시 움직인다
		void ReturnAll()
		{
			for (const auto& [id, origin] : _origins)
				_registry.Movement().AddLayeredMove(id, std::make_unique<MoveTo>(origin, ReturnDuration, EaseOut));
		}

		void KnockbackAll()
		{
			for (const auto& entry : _registry.Rects().Entries())
				_registry.Movement().AddLayeredMove(entry.id,
					std::make_unique<Impulse>(RandomVec2({ -1.f, -1.f }, { 1.f, 1.f }), KnockbackSpeed, KnockbackTime));
		}

		void ScaleSpeed(float scale)
		{
			_registry.Movement().ForEach([scale](MovementSystem::Mover& mover) { mover.velocity.speed *= scale; });
		}

		Registry									_registry;
		std::unordered_map<EntityId, Vec2>			_origins;	// 복귀 지점
		bool										_paused{};

		static constexpr std::size_t	MaxRectCount = 20;
		static constexpr float			RectSize = 0.12f;
		static constexpr float			MaxDelta = 0.05f;
		static constexpr float			ZigZagInterval = 0.3f;
		static constexpr float			ReturnDuration = 0.4f;
		static constexpr float			KnockbackSpeed = 2.0f;
		static constexpr float			KnockbackTime = 0.25f;
	};
}
