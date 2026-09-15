module;
#include <gl/glew.h>

export module hs.first_renderer;

import hs.shape;
import hs.renderer;

export namespace hs
{
    class FirstRenderer final : public IRenderer
    {
    public:
        void Clear(Color color) override
        {
            glClearColor(color.r, color.g, color.b, 1.f);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        void DrawRect(const Rectangle& rect) override
        {
            glColor3f(rect.color.r, rect.color.g, rect.color.b);
            glRectf(rect.pos.x - rect.size.x / 2, rect.pos.y - rect.size.y / 2,
                rect.pos.x + rect.size.x / 2, rect.pos.y + rect.size.y / 2);
        }
    };
}