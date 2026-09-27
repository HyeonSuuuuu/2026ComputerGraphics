module;
#include <GL/glew.h>
#include "Core/Check.h"

export module hs.first_renderer;

import std;
import hs.check;
export import hs.renderer;

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

        void Draw(Shape shape, const Transform& transform, Color color, float rotation) override
        {
            HS_DCHECK(rotation == 0.f, "FirstRenderer는 회전 없음. 회전은 ModernRenderer");
            HS_DCHECK(shape == Shape::Rect, "FirstRenderer는 사각형만. 다른 모양은 ModernRenderer");
            glColor3f(color.r, color.g, color.b);
            glRectf(transform.pos.x - transform.size.x / 2, transform.pos.y - transform.size.y / 2,
                transform.pos.x + transform.size.x / 2, transform.pos.y + transform.size.y / 2);
        }
    };
}