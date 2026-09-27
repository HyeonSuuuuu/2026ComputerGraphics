module;
#include <GLFW/glfw3.h>

export module app09;

import std;
import hs;

using namespace hs;

export namespace app09
{
	// 우클릭으로 크기를 바꿀 때 지금 방향. 손을 떼도 기억
	// 4번 스파이럴: 따라갈 경로(좌표). 화면에 선으로 그림. PathMode도 같은 점을 복사해 들고 있음
	struct SpiralPath
	{
		std::vector<Vec2> points;
	};

	struct SizeBounce
	{
		bool shrinking{};
	};

	class App09 : public App
	{
		using Super = App;
	public:
		App09(int width, int height)
			: Super(width, height, [] { return std::make_unique<ModernRenderer>(); })
			, _aspect(static_cast<float>(width) / static_cast<float>(height))
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

			// 1~4: 모든 삼각형의 이동 방식. 벽(자기 사분면)에서는 튕김
			if (input.IsKeyPressed(GLFW_KEY_1)) SetMotion(Motion::Bounce);
			if (input.IsKeyPressed(GLFW_KEY_2)) SetMotion(Motion::ZigZag);
			if (input.IsKeyPressed(GLFW_KEY_3)) SetMotion(Motion::SharpZigZag);
			if (input.IsKeyPressed(GLFW_KEY_4)) SetMotion(Motion::Spiral);

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
			FaceMovement();

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
			// 경로 선은 도형 뒤에 깔리게 먼저
			_world.Each<SpiralPath, Visual>([&](Entity, SpiralPath& path, Visual& visual)
				{
					renderer.DrawPolyline(path.points, visual.color);
				});
			RenderSystem::Draw(_world, renderer);
		}

	private:
		enum class Quadrant {TopRight, TopLeft, BottomLeft, BottomRight};
		static constexpr std::size_t QuadrantCount = 4;

		enum class Motion
		{
			Bounce,			// 대각선으로 곧게, 벽에서 튕김
			ZigZag,			// 가로로 벽까지 → 한 칸 아래 → 반대로 벽까지… 바닥에 닿으면 위로
			SharpZigZag,	// 위·아래 벽 사이를 가파른 대각선으로 오가며(V자) 옆으로. 옆 벽에서 반대로
			Spiral,			// 사분면 가운데에서 반지름이 늘며 도는 경로를 그리고 그 위를 왕복
		};

		void SetMotion(Motion motion)
		{
			_motion = motion;
			SetStatus(EnumToString(motion));
			for (std::size_t i = 0; i < QuadrantCount; ++i)
				if (Entity triangle = _world.Find(_triangles[i]))
					ApplyMotion(triangle, motion, static_cast<Quadrant>(i));
		}

		void ApplyMotion(Entity triangle, Motion motion, Quadrant quadrant)
		{
			triangle.Remove<SpiralPath>();		// 스파이럴이면 아래에서 새 경로로 다시 붙임
			Mover& mover = *triangle.Get<Mover>();
			mover.boundsResponse = BoundsResponse::Reflect;
			// 1번: 똑바로   2번: 가는 방향으로 돎(FaceMovement)   3·4번: 뒤집힌 삼각형
			const bool upsideDown = motion == Motion::SharpZigZag || motion == Motion::Spiral;
			triangle.Get<Visual>()->rotation = upsideDown ? std::numbers::pi_v<float> : 0.f;
			// 방향은 사분면 네 대각선 중 하나 → 삼각형마다 다르게 출발
			const Vec2 sign{ Random(0, 1) ? 1.f : -1.f, Random(0, 1) ? 1.f : -1.f };

			switch (motion)
			{
			case Motion::Bounce:
				mover.velocity = { .dir = Normalize(sign), .speed = MoveSpeed };
				mover.SetMode(nullptr);
				break;
			case Motion::ZigZag:
				mover.velocity = { .dir = { sign.x, 0.f }, .speed = MoveSpeed };
				// 한 칸 = 삼각형 높이 → 줄끼리 안 겹침
				mover.SetMode(std::make_unique<SweepMode>(triangle.GetTransform().size.y));
				break;
			case Motion::SharpZigZag:
				// 모드 없이 Reflect만: 위·아래 벽에서 세로가, 옆 벽에서 가로가 뒤집혀 V자가 됨
				mover.velocity = { .dir = Normalize(sign * Vec2{ 1.f, SharpZigZagSteepness }), .speed = MoveSpeed };
				mover.SetMode(nullptr);
				break;
			case Motion::Spiral:
			{
				// 경로 첫 점 = 사분면 가운데. 그 자리로 옮겨 놓고 시작(PathMode는 첫 점에서 시작해야 안 튐)
				Transform& transform = triangle.GetTransform();
				const Bounds area = BoundsOf(quadrant);
				transform.pos = (area.min + area.max) / 2.f;
				std::vector<Vec2> points = SpiralPoints(transform, area);
				mover.velocity = {};
				mover.SetMode(std::make_unique<PathMode>(points, MoveSpeed));
				triangle.Add<SpiralPath>(std::move(points));
				break;
			}
			}
		}

		// 지금 자리에서 시작해 반지름이 늘며 SpiralTurns 바퀴. 끝 반지름은 벽에 닿지 않을 만큼
		// 800×600이라 NDC에서 원을 그리면 가로로 퍼져 보임 → x를 화면 비율로 나눠 화면에서 동그랗게
		std::vector<Vec2> SpiralPoints(const Transform& transform, const Bounds& area) const
		{
			const Vec2 half = transform.size / 2.f;
			const float roomX = std::min(transform.pos.x - half.x - area.min.x, area.max.x - half.x - transform.pos.x);
			const float roomY = std::min(transform.pos.y - half.y - area.min.y, area.max.y - half.y - transform.pos.y);
			const float maxRadius = std::max(std::min(roomX * _aspect, roomY), MinSpiralRadius);

			constexpr int count = SpiralTurns * SpiralPointsPerTurn;
			std::vector<Vec2> points;
			points.reserve(count + 1);
			for (int i = 0; i <= count; ++i) {
				const float t = static_cast<float>(i) / count;
				const float angle = t * SpiralTurns * 2.f * std::numbers::pi_v<float>;
				const float radius = t * maxRadius;
				points.push_back(transform.pos + Vec2{ std::cos(angle) * radius / _aspect, std::sin(angle) * radius });
			}
			return points;
		}
		
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
			if (_motion)
				ApplyMotion(triangle, *_motion, quadrant);	// 새로 만든 것도 지금 이동 방식으로
		}

		// 2번(ZigZag)일 때만: 꼭짓점(위쪽)이 가는 방향을 향하게. 멈춰 있으면 마지막 방향 유지
		// 방향은 화면에서 보이는 각도로: NDC는 가로 1이 세로 1보다 길어서(800×600) 그대로 쓰면 비스듬히 어긋남
		void FaceMovement()
		{
			if (_motion != Motion::ZigZag)
				return;

			_world.Each<Mover, Visual>([this](Entity, Mover& mover, Visual& visual)
				{
					const Vec2 dir = mover.velocity.dir;
					if (mover.velocity.speed <= 0.f || LengthSq(dir) == 0.f)
						return;
					visual.rotation = std::atan2(dir.y, dir.x * _aspect) - std::numbers::pi_v<float> / 2.f;
				});
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
		std::optional<Motion>		_motion;					// 1~4를 누르기 전엔 제자리
		float						_aspect;					// 창 가로 / 세로. 회전 각도를 화면 기준으로

		static constexpr Color		Background{ 1.f, 1.f, 1.f };
		static constexpr Color		DividerColor{ 0.f, 0.f, 0.f };
		static constexpr float		DividerWidth = 0.006f;
		static constexpr float		MinSize = 0.05f;
		static constexpr float		TallRatio = 1.6f;		// 세로가 가로의 1.6배. 크기를 바꿔도 비율 유지(곱하기라서)
		static constexpr float		ResizePerSecond = 1.5f;	// 1초에 1.5배로 커지거나 1.5분의 1로 줄어듦

		static constexpr float		MoveSpeed = 0.4f;
		static constexpr float		SharpZigZagSteepness = 2.5f;	// 가로 1 갈 때 세로 2.5 → 뾰족한 V
		static constexpr int		SpiralTurns = 3;
		static constexpr int		SpiralPointsPerTurn = 36;		// 한 바퀴를 36개 직선으로(10도씩)
		static constexpr float		MinSpiralRadius = 0.03f;
	};

	// main에서 과제 번호만 바꾸면 되게: app09::App. 클래스 이름을 App으로 하면 hs::App과 겹침
	using App = App09;
}
