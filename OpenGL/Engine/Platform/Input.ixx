module;
#include <GLFW/glfw3.h>

export module hs.input;

import std;
export import hs.vec2;

namespace hs
{
	struct InputState
	{
		bool held{};
		bool pressed{};
		bool released{};
	};
}

export namespace hs
{
	class Input
	{
	public:
		bool IsKeyHeld(int key) const { return Get(_keys, key).held; }
		bool IsKeyPressed(int key) const { return Get(_keys, key).pressed; }
		bool IsKeyReleased(int key) const { return Get(_keys, key).released; }

		bool IsMouseHeld(int button) const { return Get(_buttons, button).held; }
		bool IsMousePressed(int button) const { return Get(_buttons, button).pressed; }
		bool IsMouseReleased(int button) const { return Get(_buttons, button).released; }

		Vec2 MousePos() const { return _mousePos; }
		Vec2 MouseDelta() const { return { _mousePos.x - _prevMousePos.x, _mousePos.y - _prevMousePos.y }; }
		float Scroll() const { return _scroll; }

		void Init(Vec2 mousePos)
		{
			_keys = {};
			_buttons = {};
			_mousePos = _prevMousePos = mousePos;
			_scroll = 0.f;
		}

		void NewFrame()
		{
			for (auto& s : _keys)    s.pressed = s.released = false;
			for (auto& s : _buttons) s.pressed = s.released = false;
			_prevMousePos = _mousePos;
			_scroll = 0.f;
		}

		void OnKey(int key, int action) { Apply(_keys, key, action); }
		void OnMouseButton(int button, int action) { Apply(_buttons, button, action); }
		void OnCursor(Vec2 pos) { _mousePos = pos; }
		void OnScroll(float offset) { _scroll += offset; }
	private:
		
		static void Apply(std::span<InputState> states, int index, int action)
		{
			if (index < 0 || index >= static_cast<int>(states.size()))
				return;

			if (action == GLFW_PRESS)
			{
				states[index].held = true;
				states[index].pressed = true;
			}
			else if (action == GLFW_RELEASE)
			{
				states[index].held = false;
				states[index].released = true;
			}
		}

		static InputState Get(std::span<const InputState> states, int index)
		{
			if (index < 0 || index >= static_cast<int>(states.size()))
				return {};
			return states[index];
		}
		
		
		std::array<InputState, GLFW_KEY_LAST + 1>	_keys{};
		std::array<InputState, GLFW_MOUSE_BUTTON_LAST + 1> _buttons{};

		Vec2 _mousePos;
		Vec2 _prevMousePos;
		float _scroll{};
	};
}