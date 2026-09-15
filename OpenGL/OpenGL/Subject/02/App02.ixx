module;
#include <gl/glfw3.h>

export module app02;

import std;
import hs;
import hs.zone;



export namespace hs
{
	class App02 : public App
	{
		using Super = App;
	public:
		App02(int width, int height)
			: Super(width, height, "App02")
			, _zones{
					  Zone({ -0.5f,  0.5f }, { 1.f, 1.f }), // 2사분면
					  Zone({  0.5f,  0.5f }, { 1.f, 1.f }), // 1사분면
					  Zone({ -0.5f, -0.5f }, { 1.f, 1.f }), // 3사분면
					  Zone({  0.5f, -0.5f }, { 1.f, 1.f }), // 4사분면
			  }
		{}

	protected:

		void Update(float dt) override
		{
			const auto& input = GetInput();

			if (input.IsKeyPressed(GLFW_KEY_1))
				_zones[0].AddItems();
			if (input.IsKeyPressed(GLFW_KEY_2))
				_zones[1].AddItems();
			if (input.IsKeyPressed(GLFW_KEY_3))
				_zones[2].AddItems();
			if (input.IsKeyPressed(GLFW_KEY_4))
				_zones[3].AddItems();

			if (input.IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
			{
				auto hit = HitTest(input.MousePos());
				_selection = (hit == _selection) ? std::nullopt : hit;
			}
			
			if (input.IsKeyPressed(GLFW_KEY_EQUAL) || input.IsKeyPressed(GLFW_KEY_KP_ADD))
			{
				if (_selection)
					_zones[_selection->zone].ResizeItem(_selection->item, 0.1f);
			}
			if (input.IsKeyPressed(GLFW_KEY_MINUS) || input.IsKeyPressed(GLFW_KEY_KP_SUBTRACT))
			{
				if (_selection)
					_zones[_selection->zone].ResizeItem(_selection->item, -0.1f);
			}
			if (input.IsKeyPressed(GLFW_KEY_C))
			{
				if (_selection)
					_zones[_selection->zone].RandomColorItem(_selection->item);
			}
			if (input.IsKeyPressed(GLFW_KEY_R))
			{
				for (auto& zone : _zones)
					zone.Init();
				_selection = std::nullopt;
			}
			if (input.IsKeyPressed(GLFW_KEY_Q))
				Close();
		}
		void Render() override
		{
			auto& renderer = GetRenderer();
			for (std::size_t i = 0; i < _zones.size(); ++i)
			{
				std::optional<RectId> selectedItem;
				if (_selection && _selection->zone == i)
					selectedItem = _selection->item;
				_zones[i].Draw(renderer, selectedItem);
			}
		}


	private:
		struct Selection
		{
			std::size_t zone;
			RectId item;
			bool operator==(const Selection&) const = default;
		};

		std::optional<Selection> HitTest(Vec2 p) const
		{
			for (std::size_t i = 0; i < _zones.size(); ++i)
				if (auto item = _zones[i].ContainItem(p))
					return Selection{ i, *item };
			return std::nullopt;
		}

		std::array<Zone, 4>			_zones;
		std::optional<Selection>	_selection;
	};
}

