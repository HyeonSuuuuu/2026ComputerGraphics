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
		App_name(int width, int height)
			: Super(width, height, "App03")
		{}

	protected:

		void Update(float dt) override
		{
			const auto& input = GetInput();
			
			if (input.IsKeyPressed(GLFW_KEY_A))
			{
				if (visibleCount <= MaxSpawnRectCount)
				{
					visibleCount++;
				}
			}
			
			if (input.IsKeyPressed(GLFW_KEY_Q))
				Close();
		}
		void Render() override
		{
			auto& renderer = GetRenderer();
			for (auto& rect : rects)
				renderer.DrawRect(rect);
		}

	private:
		static std::array<Rectangle, 20> MakeItems(const Rectangle& bg)
		{
			std::array<Rectangle, 20> items;
			for (auto& item : items)
			{
				float size = Random(MinItemSize, bg.size.x);
				item.size = { size, size };
				item.color = RandomColor();
				item.pos = bg.pos;
			}
			return items;
		}
		
		std::array<Rectangle, 20> rects;
		size_t visibleCount = 0;
		static constexpr size_t MaxSpawnRectCount = 10;
		static constexpr size_t MaxRectCount = 20;
	};
}

