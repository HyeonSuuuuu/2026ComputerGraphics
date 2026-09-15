module;
#include <gl/glfw3.h>

export module app01;

import std;
import hs;

export namespace hs
{
    class App01 : public hs::App
    {
    public:
        using Super = App;
        App01(int width, int height)
            : Super(width, height, "App01")
        {}
    protected:
        void Update(float dt) override
        {
            const auto& input = GetInput();

            if (input.IsKeyPressed(GLFW_KEY_C)) _bg = { 0.f, 1.f, 1.f };
            if (input.IsKeyPressed(GLFW_KEY_M)) _bg = { 1.f, 0.f, 1.f };
            if (input.IsKeyPressed(GLFW_KEY_Y)) _bg = { 1.f, 1.f, 0.f };
            if (input.IsKeyPressed(GLFW_KEY_G)) _bg = { 0.5f, 0.5f, 0.5f };
            if (input.IsKeyPressed(GLFW_KEY_K)) _bg = { 0.f, 0.f, 0.f };
            if (input.IsKeyPressed(GLFW_KEY_A)) _bg = RandomColor();
            if (input.IsKeyPressed(GLFW_KEY_T)) _isTimer = true;
            if (input.IsKeyPressed(GLFW_KEY_S))
            {
                _isTimer = false;
                _elapsed = 0;
            }

            if (!_isTimer)
                return;
            _elapsed += dt;
            if (_elapsed >= TimerCycleSec)
            {
                _bg = RandomColor();
                _elapsed -= TimerCycleSec;
            }
            if (input.IsKeyPressed(GLFW_KEY_Q))
                Close();
        }
        void Render() override
        {
            auto& renderer = GetRenderer();
            renderer.Clear(_bg);
        }

    private:
        hs::Color _bg{ 1.f, 1.f, 1.f };
        bool _isTimer{};
        float _elapsed{};

        static constexpr float TimerCycleSec = 0.5f;
    };
}