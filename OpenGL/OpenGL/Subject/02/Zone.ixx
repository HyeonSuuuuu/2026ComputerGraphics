export module hs.zone;

import std;
import hs.shape;
import hs.renderer;
import hs.random;

export namespace hs
{
	class Zone
	{
	public:
		Zone(Vec2 center, Vec2 size)
			: _bg{ center, size, RandomColor() }
			, _items {MakeItems(_bg)}
		{}
		
		void Draw(IRenderer& renderer, std::optional<std::size_t> selectedItem = std::nullopt) const
		{
			renderer.DrawRect(_bg);
			for (auto i : std::views::iota(0uz, _visibleCount))
			{
				if (i == selectedItem)
					DrawSelected(renderer, _items[i]);
				else
					renderer.DrawRect(_items[i]);
			}
		}

		bool Contains(Vec2 p) const { return _bg.Contains(p); }

		std::optional<std::size_t> ContainItem(Vec2 p) const
		{
			for (auto i : std::views::iota(0uz, _visibleCount) | std::views::reverse)
				if (_items[i].Contains(p))
					return i;
			return std::nullopt;
		}

		bool AddItems()
		{
			if (_visibleCount >= _items.size())
				return false;
			
			++_visibleCount;
			return true;
		}
		
		void ResizeItem(std::optional<std::size_t> selectedItem = std::nullopt, float delta = 0.1f)
		{
			if (selectedItem)
			{
				size_t index = selectedItem.value();
				Vec2& size = _items[index].size;
				
				size.x = std::clamp(size.x + delta, MinItemSize, _bg.size.x);
				size.y = std::clamp(size.y + delta, MinItemSize, _bg.size.y);
			}
		}
		
		void RandomColorItem(std::optional<std::size_t> selectedItem = std::nullopt)
		{
			if (selectedItem)
			{
				size_t index = selectedItem.value();
				_items[index].color = RandomColor();
			}
		}
		
		void Init()
		{
			_visibleCount = 0;
			_bg.color = RandomColor();
			for (auto& item : _items)
			{
				float size = Random(MinItemSize, _bg.size.x);
				item.size = { size, size };
				item.color = RandomColor();
				item.pos = _bg.pos;
			}
		}
		
	private:
		static std::array<Rectangle, 5> MakeItems(const Rectangle& bg)
		{
			std::array<Rectangle, 5> items;
			for (auto& item : items)
			{
				float size = Random(MinItemSize, bg.size.x);
				item.size = { size, size };
				item.color = RandomColor();
				item.pos = bg.pos;
			}
			return items;
		}

		static void DrawSelected(IRenderer& renderer, const Rectangle& rect)
		{
			renderer.DrawRect({ rect.pos, rect.size, SelectedColor });
			renderer.DrawRect({ rect.pos, { rect.size.x - Border * 2, rect.size.y - Border * 2}, rect.color});
		}

		Rectangle					_bg;
		std::array<Rectangle, 5>	_items;
		std::size_t					_visibleCount{};

		static constexpr float Border = 0.02f;
		static constexpr Color SelectedColor{ 1.f, 1.f, 0.f };
		
		static constexpr float MinItemSize = 0.01f;
	};
}

