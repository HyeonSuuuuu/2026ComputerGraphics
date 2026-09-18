module;
#include <gl/glm/glm.hpp>

export module hs.movement_system;

import std;
import hs.check;
import hs.shape;
import hs.entity;
import hs.movement;
import hs.rect_layer;

export namespace hs
{
	class MovementSystem
	{
	public:
		// 이동 주체 하나. 사각형 하나에 대응한다
		struct Mover
		{
			Velocity velocity;
			BoundsResponse bounds{ BoundsResponse::Reflect };
			bool enabled{ true };

			std::unique_ptr<IMovementMode>				  mode;		// 배타적. 하나만
			std::vector<std::unique_ptr<ILayeredMove>> layeredMoves;	// 겹쳐 쌓임. 끝나면 제거됨
		};

		Mover& Attach(EntityId id, Velocity velocity, BoundsResponse bounds = BoundsResponse::Reflect)
		{
			Check(IsValidDirection(velocity.dir), "dir은 Normalize해서 넘겨야 한다");
			Check(velocity.speed >= 0.f, "속력은 음수가 될 수 없다. 반대로 가려면 dir을 뒤집어라");

			Mover& mover = _movers[id];
			mover.velocity = velocity;
			mover.bounds = bounds;
			mover.enabled = true;
			mover.mode.reset();
			mover.layeredMoves.clear();
			return mover;
		}

		void SetBounds(Bounds bounds)
		{
			Check(bounds.min.x <= bounds.max.x && bounds.min.y <= bounds.max.y, "bounds의 min과 max가 뒤집혀 있다");
			_bounds = bounds;
		}

		Bounds GetBounds() const { return _bounds; }

		void Detach(EntityId id) { _movers.erase(id); }
		void Clear() { _movers.clear(); }

		Mover* Find(EntityId id)
		{
			auto it = _movers.find(id);
			return it != _movers.end() ? &it->second : nullptr;
		}

		void SetMode(EntityId id, std::unique_ptr<IMovementMode> mode)
		{
			Mover* mover = Find(id);
			Check(mover, "Attach하지 않은 대상에 모드를 넣으려 한다");
			if (mover)
				mover->mode = std::move(mode);	// 기존 모드는 여기서 소멸
		}

		void AddLayeredMove(EntityId id, std::unique_ptr<ILayeredMove> move)
		{
			Check(move != nullptr, "빈 LayeredMove를 넣으면 Tick에서 터진다");

			Mover* mover = Find(id);
			Check(mover, "Attach하지 않은 대상에 LayeredMove를 넣으려 한다");
			if (mover)
				mover->layeredMoves.push_back(std::move(move));
		}

		// 전체에 적용할 때 사용 (속도 조절, 일시정지 등)
		void ForEach(std::invocable<Mover&> auto fn)
		{
			for (auto& [id, mover] : _movers)
				fn(mover);
		}

		void Tick(RectLayer& layer, float dt)
		{
			Check(dt >= 0.f, "시간은 거꾸로 흐르지 않는다");
						for (auto it = _movers.begin(); it != _movers.end();)
			{
				Rectangle* rect = layer.Find(it->first);
				if (!rect)						// 사각형이 사라졌으면 Mover도 정리
				{
					it = _movers.erase(it);
					continue;
				}

				Mover& mover = it->second;
				if (mover.enabled)
					Step(mover, *rect, dt, _bounds);
				++it;
			}
		}

	private:
		static void Step(Mover& mover, Rectangle& rect, float dt, const Bounds& bounds)
		{
			// 1. 이동 방식이 속도를 정한다
			if (mover.mode)
				mover.mode->CalcVelocity(mover.velocity, rect, dt);

			// 2. 얹힌 LayeredMove들을 섞어 이번 프레임의 속도를 만든다
			Vec2 velocity = mover.velocity.Value();
			bool overridden = false;
			for (auto& move : mover.layeredMoves)
			{
				Vec2 value = move->Evaluate(rect, dt);
				if (move->Mode() == MixMode::Override)
				{
					velocity = value;
					overridden = true;
				}
				else
				{
					velocity += value;
				}
			}
			std::erase_if(mover.layeredMoves, [](const auto& move) { return move->IsFinished(); });

			// NaN은 한 번 들어오면 계속 퍼져서 사각형이 화면에서 사라진다. 여기서 잡는다
			Check(std::isfinite(velocity.x) && std::isfinite(velocity.y), "속도가 NaN이나 무한이다");

			// 3. 적분. 위치를 바꾸는 유일한 지점
			rect.pos += velocity * dt;

			// 4. 경계 처리
			// Override 중에는 Reflect를 적용하지 않는다. LayeredMove가 정한 경로 때문에
			// 지속 속도의 방향이 뒤집히면, 그 이동이 끝난 뒤 엉뚱한 방향으로 움직인다
			ResolveBounds(rect, mover.velocity,
				overridden ? BoundsResponse::Clamp : mover.bounds, bounds);
		}

		// 축마다 같은 처리를 반복한다. 3D로 갈 때는 AxisCount만 3으로 바꾸면 된다
		static constexpr int AxisCount = 2;

		static void ResolveBounds(Rectangle& rect, Velocity& velocity, BoundsResponse response, const Bounds& bounds)
		{
			if (response == BoundsResponse::None)
				return;

			Vec2 half = rect.size / 2.f;

			for (int axis = 0; axis < AxisCount; ++axis)
			{
				if (response == BoundsResponse::Wrap)
				{
					// 완전히 빠져나간 뒤에 반대편으로 보낸다. 걸치자마자 옮기면 튀어 보인다
					if (rect.pos[axis] + half[axis] < bounds.min[axis])
						rect.pos[axis] = bounds.max[axis] + half[axis];
					else if (rect.pos[axis] - half[axis] > bounds.max[axis])
						rect.pos[axis] = bounds.min[axis] - half[axis];
					continue;
				}

				float low = bounds.min[axis] + half[axis];		// 중심이 가질 수 있는 범위
				float high = bounds.max[axis] - half[axis];
				if (low > high)									// 경계보다 큰 사각형은 밀어낼 곳이 없다
					continue;

				// Clamp와 Reflect는 되돌리는 처리가 같고, 방향 반전 여부만 다르다
				bool reflect = (response == BoundsResponse::Reflect);

				if (rect.pos[axis] < low)
				{
					rect.pos[axis] = low;
					// 부호를 뒤집는 대신 안쪽 방향으로 맞춘다. 한 프레임에 빠져나오지 못해도 끼지 않는다
					if (reflect) velocity.dir[axis] = std::abs(velocity.dir[axis]);
				}
				else if (rect.pos[axis] > high)
				{
					rect.pos[axis] = high;
					if (reflect) velocity.dir[axis] = -std::abs(velocity.dir[axis]);
				}
			}
		}

		// 움직일 수 있는 영역. 레벨이 바뀔 때만 갱신하면 된다
		Bounds _bounds{ .min{ -1.f, -1.f }, .max{ 1.f, 1.f } };	// 기본값은 NDC 전체
		std::unordered_map<EntityId, Mover> _movers;
	};
}
