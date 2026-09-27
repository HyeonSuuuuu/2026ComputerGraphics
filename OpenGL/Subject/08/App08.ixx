module;
#include <GLFW/glfw3.h>

export module app08;

import std;
import hs;

using namespace hs;

export namespace app08
{
	// 우클릭으로 크기를 바꿀 때 지금 방향. 손을 떼도 기억
	struct SizeBounce
	{
		bool shrinking{};
	};

	class App08 : public App
	{
		using Super = App;
	public:
		App08(int width, int height)
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

			if (input.IsKeyPressed(GLFW_KEY_A))
				_drawMode = DrawMode::Fill;
			if (input.IsKeyPressed(GLFW_KEY_B))
				_drawMode = DrawMode::Line;

			// C: 사분면마다 지우고 새로
			if (input.IsKeyPressed(GLFW_KEY_C))
				for (std::size_t i = 0; i < QuadrantCount; ++i) {
					Quadrant quadrant = static_cast<Quadrant>(i);
					Bounds area = BoundsOf(quadrant);
					SpawnTriangle(quadrant, RandomVec2(area.min, area.max));
				}

			// 왼쪽 클릭: 그 사분면에 새로 (있던 것은 지움)
			if (input.IsMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
				Vec2 mouse = input.MousePos();
				SpawnTriangle(QuadrantOf(mouse), mouse);
			}
			
			_world.Flush();						// 목록 확정. 이번 프레임 생성분도 아래 시스템 대상

			EffectSystem::Tick(_world, dt);
			MovementSystem::Tick(_world, dt);
			CollisionSystem::Tick(_world);

			// 오른쪽 누르는 동안만: 마우스가 있는 사분면의 삼각형 크기가 오감
			// hitBounds는 방금 CollisionSystem이 채운 값(지난 프레임에 키운 결과) → Tick 뒤에
			if (input.IsMouseHeld(GLFW_MOUSE_BUTTON_RIGHT))
				if (Entity triangle = _world.Find(_triangles[static_cast<std::size_t>(QuadrantOf(input.MousePos()))]))
					Resize(triangle, dt);
		}

		void Render() override
		{
			auto& renderer = GetRenderer();
			renderer.Clear(Background);
			renderer.SetDrawMode(DrawMode::Fill);		// 분할선은 A/B와 무관하게 늘 면
			renderer.Draw(Shape::Rect, { .pos = {}, .size = { DividerWidth, 2.f } }, DividerColor);
			renderer.Draw(Shape::Rect, { .pos = {}, .size = { 2.f, DividerWidth } }, DividerColor);
			renderer.SetDrawMode(_drawMode);
			RenderSystem::Draw(_world, renderer);
		}

	private:
		enum class Quadrant {TopRight, TopLeft, BottomLeft, BottomRight};
		static constexpr std::size_t QuadrantCount = 4;
		
		// 사분면마다 삼각형은 하나 → 있던 것은 지우고 새로
		void SpawnTriangle(Quadrant quadrant, Vec2 pos)
		{
			EntityId& slot = _triangles[static_cast<std::size_t>(quadrant)];
			if (Entity old = _world.Find(slot))
				_world.Destroy(old);

			const float width = Random(0.15f, 0.3f);
			Entity triangle = _world.Spawn({ .pos = pos, .size = { width, width * TallRatio } });
			triangle.Add<Visual>(RandomColor()).shape = Shape::Triangle;
			// 움직이지 않아도 Mover가 있어야 경계 처리 대상. 경계 근처에 찍히거나 커져도 자기 사분면 안으로 밀림
			triangle.Add<Mover>(Velocity{ .dir = {} }, BoundsResponse::Clamp);
			triangle.Add<Confine>(BoundsOf(quadrant));
			triangle.Add<SizeBounce>();
			slot = triangle.Id();
		}

		// 커지다 경계에 닿으면 줄고, 거의 사라지면(MinSize) 다시 커짐
		static void Resize(Entity triangle, float dt)
		{
			SizeBounce& bounce = *triangle.Get<SizeBounce>();
			Vec2& size = triangle.GetTransform().size;

			if (triangle.Get<Mover>()->hitBounds)
				bounce.shrinking = true;
			else if (size.x <= MinSize || size.y <= MinSize)
				bounce.shrinking = false;

			// 초당 배율. 줄 때는 역수 → 커지는 속도와 줄어드는 속도가 대칭
			size *= std::pow(ResizePerSecond, bounce.shrinking ? -dt : dt);
			size = Max(size, Vec2{ MinSize });
		}

		static Quadrant QuadrantOf(Vec2 p)
		{
			if (p.y >= 0.f) return p.x >= 0.f ? Quadrant::TopRight : Quadrant::TopLeft;
			return p.x < 0.f ? Quadrant::BottomLeft : Quadrant::BottomRight;
		}

		// 분할선 두께의 절반만큼 안쪽
		static Bounds BoundsOf(Quadrant quadrant)
		{
			constexpr float gap = DividerWidth / 2.f;
			switch (quadrant)
			{
			case Quadrant::TopRight:    return { .min{ gap, gap },   .max{ 1.f, 1.f } };
			case Quadrant::TopLeft:     return { .min{ -1.f, gap },  .max{ -gap, 1.f } };
			case Quadrant::BottomLeft:  return { .min{ -1.f, -1.f }, .max{ -gap, -gap } };
			case Quadrant::BottomRight: return { .min{ gap, -1.f },  .max{ 1.f, -gap } };
			}
			std::unreachable();
		}
		
		std::array<EntityId, QuadrantCount>	_triangles {};
		World						_world;
		DrawMode					_drawMode = DrawMode::Fill;	// A: 면, B: 선

		static constexpr Color		Background{ 1.f, 1.f, 1.f };
		static constexpr Color		DividerColor{ 0.f, 0.f, 0.f };
		static constexpr float		DividerWidth = 0.006f;
		static constexpr float		MinSize = 0.05f;
		static constexpr float		TallRatio = 1.6f;		// 세로가 가로의 1.6배. 크기를 바꿔도 비율 유지(곱하기라서)
		static constexpr float		ResizePerSecond = 1.5f;	// 1초에 1.5배로 커지거나 1.5분의 1로 줄어듦
	};

	// main에서 과제 번호만 바꾸면 되게: app09::App. 클래스 이름을 App으로 하면 hs::App과 겹침
	using App = App08;
}
