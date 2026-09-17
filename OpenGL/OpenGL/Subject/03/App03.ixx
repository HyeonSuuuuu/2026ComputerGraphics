module;
#include <gl/glfw3.h>

export module app03;

import std;
import hs;



export namespace hs
{
	class App03 : public App
	{
		using Super = App;
	public:
		App03(int width, int height)
			: Super(width, height, "App03")
		{}

	protected:

		void Update(float dt) override
		{
			const auto& input = GetInput();

			if (input.IsKeyPressed(GLFW_KEY_A))
				SpawnRect();
			if (input.IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
				_dragging = _rects.HitTest(input.MousePos());
			if (_dragging)
				DragRect(*_dragging, input.MouseDelta());
			if (input.IsMouseReleased(GLFW_MOUSE_BUTTON_LEFT))
			{
				if (_dragging)
					DropRect(*_dragging);
				_dragging.reset();
			}
			if (input.IsMousePressed(GLFW_MOUSE_BUTTON_RIGHT))
				if (auto hit = _rects.HitTest(input.MousePos()))
					SplitRect(*hit);

			if (input.IsKeyPressed(GLFW_KEY_Q))
				Close();
		}
		void Render() override
		{
			auto& renderer = GetRenderer();
			renderer.Clear({ 1.f, 1.f, 1.f });
			for (const auto& entry : _rects.Entries())
				renderer.DrawRect(entry.rect);
		}

	private:
		void SpawnRect()
		{
			if (_rects.Size() >= MaxSpawnRectCount)
				return;

			float w = Random(MinRectSize, MaxSpawnRectSize);
			float h = Random(MinRectSize, MaxSpawnRectSize);
			Vec2 pos{ Random(-1.f + w / 2, 1.f - w / 2), Random(-1.f + h / 2, 1.f - h / 2) };

			_rects.Add({ pos, { w, h }, RandomColor()});
		}

		void DragRect(RectId id, Vec2 delta)
		{
			Rectangle* rect = _rects.Find(id);
			if (!rect)
			{
				_dragging.reset();
				return;
			}

			rect->pos.x += delta.x;
			rect->pos.y += delta.y;
		}
		
		void DropRect(RectId id)
		{
			if (auto other = _rects.FindOverlap(id))
				MergeRects(id, *other);
		}
		
		RectId MergeRects(RectId a, RectId b)
		{
			const Rectangle& ra = *_rects.Find(a);
			const Rectangle& rb = *_rects.Find(b);
			
			float areaA = ra.size.x * ra.size.y;
			float areaB = rb.size.x * rb.size.y;
			float area = areaA + areaB;
			
			float aspect = (ra.size.x + rb.size.x) / (ra.size.y + rb.size.y);
			float width = std::sqrt(area * aspect);
			float height = area / width;
			
			Vec2 pos{
				(ra.pos.x * areaA + rb.pos.x * areaB) / area,
				(ra.pos.y * areaA + rb.pos.y * areaB) / area
		};

			Rectangle merged{ pos, { width, height }, RandomColor() };
			
			
			_rects.Remove(a);
			_rects.Remove(b);
			return _rects.Add(merged);
		}

		void SplitRect(RectId id)
		{
			if (_rects.Size() >= MaxRectCount)
				return;

			Rectangle original = *_rects.Find(id);	
			_rects.Remove(id);
			_rects.Add(MakeRectInside(original));
			_rects.Add(MakeRectInside(original));
		}
		
		static Rectangle MakeRectInside(const Rectangle& bounds)
		{
			float w = Random(MinRectSize, bounds.size.x);
			float h = Random(MinRectSize, bounds.size.y);
			Vec2 pos{
				Random(bounds.Left() + w / 2,   bounds.Right() - w / 2),
				Random(bounds.Bottom() + h / 2, bounds.Top() - h / 2)
			};
			return { pos, { w, h }, RandomColor() };
		}

		RectLayer						_rects;
		std::optional<RectId>			_dragging;

		static constexpr std::size_t	MaxSpawnRectCount = 10;
		static constexpr std::size_t	MaxRectCount = 20;
		static constexpr float			MinRectSize = 0.05f;
		static constexpr float			MaxSpawnRectSize = 0.4f;
	};
}

