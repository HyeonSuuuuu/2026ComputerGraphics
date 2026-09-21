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

        void DrawRect(const Transform& transform, Color color) override
        {
            glColor3f(color.r, color.g, color.b);
            glRectf(transform.pos.x - transform.size.x / 2, transform.pos.y - transform.size.y / 2,
                transform.pos.x + transform.size.x / 2, transform.pos.y + transform.size.y / 2);
        }
    };
}