module;
#include <GLFW/glfw3.h>

export module app09;

import std;
import hs;

using namespace hs;

export namespace app09
{
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

			// 1~4: 이동 방식. 화면 끝에서는 튕김
			if (input.IsKeyPressed(GLFW_KEY_1)) SetMotion(Motion::Bounce);
			if (input.IsKeyPressed(GLFW_KEY_2)) SetMotion(Motion::ZigZag);
			if (input.IsKeyPressed(GLFW_KEY_3)) SetMotion(Motion::SharpZigZag);
			if (input.IsKeyPressed(GLFW_KEY_4)) SetMotion(Motion::Spiral);

			if (input.IsKeyPressed(GLFW_KEY_A))
				_drawMode = DrawMode::Fill;
			if (input.IsKeyPressed(GLFW_KEY_B))
				_drawMode = DrawMode::Line;

			// C: 지우고 랜덤 위치에 새로
			if (input.IsKeyPressed(GLFW_KEY_C)) {
				const Bounds area = _world.WorldBounds();
				SpawnTriangle(RandomVec2(area.min, area.max));
			}

			// 왼쪽 클릭: 지우고 그 자리에 새로
			if (input.IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
				SpawnTriangle(input.MousePos());

			_world.Flush();						// 목록 확정. 이번 프레임 생성분도 아래 시스템 대상

			EffectSystem::Tick(_world, dt);
			MovementSystem::Tick(_world, dt);
			CollisionSystem::Tick(_world);
			FaceMovement();

			// 오른쪽 누르는 동안만: 크기가 오감
			// hitBounds는 방금 CollisionSystem이 채운 값(지난 프레임에 키운 결과) → Tick 뒤에
			if (input.IsMouseHeld(GLFW_MOUSE_BUTTON_RIGHT))
				if (Entity triangle = _world.Find(_triangle))
					Resize(triangle, dt);
		}

		void Render() override
		{
			auto& renderer = GetRenderer();
			renderer.Clear(Background);
			renderer.SetDrawMode(_drawMode);
			// 경로 선은 도형 뒤에 깔리게 먼저
			_world.Each<SpiralPath, Visual>([&](Entity, SpiralPath& path, Visual& visual)
				{
					renderer.DrawPolyline(path.points, visual.color);
				});
			RenderSystem::Draw(_world, renderer);
		}

	private:
		enum class Motion
		{
			Bounce,			// 대각선으로 곧게, 벽에서 튕김
			ZigZag,			// 가로로 벽까지 → 한 칸 아래 → 반대로 벽까지… 바닥에 닿으면 위로
			SharpZigZag,	// 위·아래 벽 사이를 가파른 대각선으로 오가며(V자) 옆으로. 옆 벽에서 반대로
			Spiral,			// 화면 가운데에서 반지름이 늘며 도는 경로를 그리고 그 위를 왕복
		};

		void SetMotion(Motion motion)
		{
			_motion = motion;
			SetStatus(EnumToString(motion));
			if (Entity triangle = _world.Find(_triangle))
				ApplyMotion(triangle, motion);
		}

		void ApplyMotion(Entity triangle, Motion motion)
		{
			triangle.Remove<SpiralPath>();		// 스파이럴이면 아래에서 새 경로로 다시 붙임
			Mover& mover = *triangle.Get<Mover>();
			mover.boundsResponse = BoundsResponse::Reflect;
			// 1번: 똑바로   2번: 가는 방향으로 돎(FaceMovement)   3·4번: 뒤집힌 삼각형
			const bool upsideDown = motion == Motion::SharpZigZag || motion == Motion::Spiral;
			triangle.Get<Visual>()->rotation = upsideDown ? std::numbers::pi_v<float> : 0.f;
			// 출발 방향은 네 대각선 중 하나
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
				// 화면 가운데로 보내놓고 시작 (PathMode는 첫 점에서 시작해야 안 튐)
				Transform& transform = triangle.GetTransform();
				const Bounds area = _world.WorldBounds();
				transform.pos = (area.min + area.max) / 2.f;
				std::vector<Vec2> points = SpiralPoints(transform, area);
				mover.velocity = {};
				mover.SetMode(std::make_unique<PathMode>(points, MoveSpeed));
				triangle.Add<SpiralPath>(std::move(points));
				break;
			}
			}
		}

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

		// 삼각형은 하나 → 있던 것은 지우고 새로
		void DestroyTriangle(Entity old)
		{
			_world.Destroy(old);
		}

		void SpawnTriangle(Vec2 pos)
		{
			if (Entity old = _world.Find(_triangle))
				DestroyTriangle(old);

			const float width = Random(0.15f, 0.3f);
			Entity triangle = _world.Spawn({ .pos = pos, .size = { width, width * TallRatio } });
			triangle.Add<Visual>(RandomColor()).shape = Shape::Triangle;
			// 움직이지 않아도 Mover가 있어야 경계 처리 대상. 화면 끝에 찍히거나 커져도 안으로 밀림
			triangle.Add<Mover>(Velocity{ .dir = {} }, BoundsResponse::Clamp);
			triangle.Add<SizeBounce>();
			_triangle = triangle.Id();
			if (_motion)
				ApplyMotion(triangle, *_motion);	// 새로 만든 것도 지금 이동 방식으로
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

			// Bounce
			size *= std::pow(ResizePerSecond, bounce.shrinking ? -dt : dt);
			size = Max(size, Vec2{ MinSize });
		}

		EntityId					_triangle{};
		World						_world;
		DrawMode					_drawMode = DrawMode::Fill;	// A: 면, B: 선
		std::optional<Motion>		_motion;					// 1~4를 누르기 전엔 제자리
		float						_aspect;

		static constexpr Color		Background{ 1.f, 1.f, 1.f };
		static constexpr float		MinSize = 0.05f;
		static constexpr float		TallRatio = 1.6f;
		static constexpr float		ResizePerSecond = 1.5f;	// 1초에 1.5배로 커지거나 1.5분의 1로 줄어듦

		static constexpr float		MoveSpeed = 0.4f;
		static constexpr float		SharpZigZagSteepness = 3.5f;
		static constexpr int		SpiralTurns = 3;
		static constexpr int		SpiralPointsPerTurn = 36;
		static constexpr float		MinSpiralRadius = 0.03f;
	};

	using App = App09;
}
