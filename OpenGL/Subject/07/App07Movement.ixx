module;
#include <GLFW/glfw3.h>

export module app07.movement;

import std;
import hs;

using namespace hs;

export namespace app07
{
	class KeyboardMode : public IMovementMode
	{
	public:
		explicit KeyboardMode(const Input& input) : _input(input) {}

		void CalcVelocity(Velocity& velocity, const Transform&, float) override
		{
			Vec2 dir{};
			if (_input.IsKeyHeld(GLFW_KEY_W)) dir.y += 1.f;
			if (_input.IsKeyHeld(GLFW_KEY_S)) dir.y -= 1.f;
			if (_input.IsKeyHeld(GLFW_KEY_A)) dir.x -= 1.f;
			if (_input.IsKeyHeld(GLFW_KEY_D)) dir.x += 1.f;
			
			if (_input.IsKeyHeld(GLFW_KEY_I)) // 좌상
			{
				dir.x -= 1.f;
				dir.y += 1.f;
			}
			if (_input.IsKeyHeld(GLFW_KEY_J)) // 우상
			{
				dir.x += 1.f;
				dir.y += 1.f;
			}
			if (_input.IsKeyHeld(GLFW_KEY_K)) // 좌하
			{
				dir.x -= 1.f;
				dir.y -= 1.f;
			}
			if (_input.IsKeyHeld(GLFW_KEY_L)) // 우하
			{
				dir.x += 1.f;
				dir.y -= 1.f;
			}
			
			velocity.dir = Normalize(dir);
		}

	private:
		const Input& _input;
	};
}
