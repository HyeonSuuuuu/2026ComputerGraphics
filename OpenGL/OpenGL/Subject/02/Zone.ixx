export module hs.zone;

import std;
import hs.shape;
import hs.renderer;
import hs.random;
import hs.rect_layer;

export namespace hs
{
	class Zone
	{
	public:
		Zone(Vec2 center, Vec2 size)
			: _bg{ center, size, RandomColor() }
		{}

		void Draw(IRenderer& renderer, std::optional<RectId> selectedItem = std::nullopt) const
		{
			renderer.DrawRect(_bg);
			for (const auto& entry : _items.Entries())
			{
				if (entry.id == selectedItem)
					DrawSelected(renderer, entry.rect);
				else
					renderer.DrawRect(entry.rect);
			}
		}

		bool Contains(Vec2 p) const { return _bg.Contains(p); }

		std::optional<RectId> ContainItem(Vec2 p) const
		{
			// 배경 밖이면 아이템은 검사할 필요 없음
			if (!_bg.Contains(p))
				return std::nullopt;
			return _items.HitTest(p);
		}

		bool AddItems()
		{
			if (_items.Size() >= MaxItemCount)
				return false;

			_items.Add(MakeItem(_bg));
			return true;
		}

		void ResizeItem(RectId id, float delta)
		{
			if (Rectangle* item = _items.Find(id))
			{
				item->size.x = std::clamp(item->size.x + delta, MinItemSize, _bg.size.x);
				item->size.y = std::clamp(item->size.y + delta, MinItemSize, _bg.size.y);
			}
		}

		void RandomColorItem(RectId id)
		{
			if (Rectangle* item = _items.Find(id))
				item->color = RandomColor();
		}

		void Init()
		{
			_bg.color = RandomColor();
			_items.Clear();
		}

	private:
		static Rectangle MakeItem(const Rectangle& bg)
		{
			float size = Random(MinItemSize, bg.size.x);
			return { bg.pos, { size, size }, RandomColor() };
		}

		static void DrawSelected(IRenderer& renderer, const Rectangle& rect)
		{
			renderer.DrawRect({ rect.pos, rect.size, SelectedColor });
			renderer.DrawRect({ rect.pos, { rect.size.x - Border * 2, rect.size.y - Border * 2}, rect.color});
		}

		Rectangle	_bg;
		RectLayer	_items;

		static constexpr std::size_t MaxItemCount = 5;
		static constexpr float Border = 0.02f;
		static constexpr Color SelectedColor{ 1.f, 1.f, 0.f };

		static constexpr float MinItemSize = 0.01f;
	};
}

