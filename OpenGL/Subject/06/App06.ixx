module;
#include <GLFW/glfw3.h>

export module app06;

import std;
import hs;
import app06.rect;

using namespace hs;

export namespace app06
{
	class App06 : public App
	{
		using Super = App;
	public:
		App06(int width, int height)
			: Super(width, height)
		{
			const int count = Random(5, 10);
			for (int i = 0; i < count; ++i)
				Rect::Spawn(_world
					, RandomVec2({ -0.9f, -0.9f }, { 0.9f, 0.9f })
					, RandomVec2({0.12, 0.12}, {0.4, 0.4}));
			
			// 맵 범위가 화면과 다를 때만
			// _world.SetBounds({ .min{ -1.f, -1.f }, .max{ 1.f, 1.f } });
		}

	protected:

		void Update(float dt) override
		{
			const auto& input = GetInput();

			if (input.IsKeyPressed(GLFW_KEY_Q))
				Close();
		
			if (input.IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
				if (Entity hit = _world.HitTest(input.MousePos()))
					Split(hit);

			_world.Flush();						// 목록 확정. 이번 프레임 생성분도 아래 시스템 대상

			EffectSystem::Tick(_world, dt);
			MovementSystem::Tick(_world, dt);
			CollisionSystem::Separate(_world);
			CollisionSystem::Tick(_world);
		}

		void Render() override
		{
			auto& renderer = GetRenderer();
			renderer.Clear(Background);
			RenderSystem::Draw(_world, renderer);
		}

	private:
		enum class Motion
		{
			Cross,		// ① 상하좌우로 흩어짐
			Diagonal,	// ② 대각선으로 흩어짐
			Together,	// ③ 넷이 같은 방향으로
			EightWay,	// ④ 8조각이 8방향으로
		};

		// offset: 원본 크기 대비 조각 중심 위치
		struct Piece
		{
			Vec2 offset;
			Vec2 dir;
		};

		struct SplitPattern
		{
			float scale;				// 원본 대비 조각 크기
			std::vector<Piece> pieces;
		};

		static SplitPattern PatternOf(Motion motion)
		{
			constexpr Vec2 TopLeft{ -0.25f, 0.25f }, TopRight{ 0.25f, 0.25f };
			constexpr Vec2 BottomLeft{ -0.25f, -0.25f }, BottomRight{ 0.25f, -0.25f };

			switch (motion)
			{
			case Motion::Cross:
				// 각 조각이 자기 쪽 바깥 변 방향으로
				return { 0.5f, { { TopLeft, { -1.f, 0.f } }, { TopRight, { 0.f, 1.f } },
					{ BottomRight, { 1.f, 0.f } }, { BottomLeft, { 0.f, -1.f } } } };

			case Motion::Diagonal:
				return { 0.5f, { { TopLeft, Normalize(TopLeft) }, { TopRight, Normalize(TopRight) },
					{ BottomLeft, Normalize(BottomLeft) }, { BottomRight, Normalize(BottomRight) } } };

			case Motion::Together:
			{
				Vec2 dir = EightDirections()[Random(0, 7)];
				return { 0.5f, { { TopLeft, dir }, { TopRight, dir }, { BottomLeft, dir }, { BottomRight, dir } } };
			}

			case Motion::EightWay:
			{
				// 3×3에서 가운데를 뺀 8칸, 각자 바깥으로
				SplitPattern pattern{ 1.f / 3.f, {} };
				for (Vec2 dir : EightDirections())
					pattern.pieces.push_back({ Vec2{ std::round(dir.x), std::round(dir.y) } / 3.f, dir });
				return pattern;
			}
			}
			return {};
		}

		static std::array<Vec2, 8> EightDirections()
		{
			const float d = std::sqrt(0.5f);
			return { { { 1.f, 0.f }, { d, d }, { 0.f, 1.f }, { -d, d },
				{ -1.f, 0.f }, { -d, -d }, { 0.f, -1.f }, { d, -d } } };
		}

		void Split(Entity target)
		{
			// Spawn이 저장소를 재배치하므로 값으로 복사
			const Transform origin = target.GetTransform();
			const Color color = target.Get<Visual>()->color;
			_world.Destroy(target);

			if (std::min(origin.size.x, origin.size.y) < Rect::MinSize)
				return;

			const auto motion = static_cast<Motion>(Random(0, static_cast<int>(EnumCount<Motion>) - 1));
			const SplitPattern pattern = PatternOf(motion);
			const Bounds area = _world.WorldBounds();

			for (const Piece& piece : pattern.pieces)
			{
				Transform transform{ .pos = origin.pos + piece.offset * origin.size, .size = origin.size * pattern.scale };

				// 화면 밖 목표는 영영 도착 못 함 → 안쪽으로
				Vec2 goal = transform.pos + piece.dir * Rect::MoveDistance;
				Vec2 half = transform.size / 2.f;
				goal.x = std::clamp(goal.x, area.min.x + half.x, area.max.x - half.x);
				goal.y = std::clamp(goal.y, area.min.y + half.y, area.max.y - half.y);

				Rect::SpawnPiece(_world, transform, color, goal);
			}
		}
		
		World						_world;

		static constexpr Color		Background{ 1.f, 1.f, 1.f };
	};
}
