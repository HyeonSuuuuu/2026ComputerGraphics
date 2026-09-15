module;
#include <gl/glfw3.h>

export module app_name;

import std;
import hs;



export namespace hs
{
	class App_name : public App
	{
		using Super = App;
	public:
		App_name(int width, int height)
			: Super(width, height, "app_name")
		{}

	protected:

		void Update(float dt) override
		{
			const auto& input = GetInput();
			
			if (input.IsKeyPressed(GLFW_KEY_Q))
				Close();
		}
		void Render() override
		{
			auto& renderer = GetRenderer();
		}

	private:
	};
}

