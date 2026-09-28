export module hs.render_system;

import std;
import hs.transform;
export import hs.renderer;
import hs.visual;
export import hs.world;

export namespace hs
{
	// 그리는 순서 = 칸 순서(Objects). Each는 삭제 후 순서가 섞여서 안 씀
	class RenderSystem
	{
	public:
		static void Draw(World& world, IRenderer& renderer)
		{
			for (Entity entity : world.Entities())
				if (const Visual* visual = entity.Get<Visual>())
				{
					Transform drawn = entity.GetTransform();
					drawn.size *= visual->scale * visual->effectScale;		// 연출 배율은 그릴 때만
					// 테두리: 테두리 색으로 전체를 칠하고, 그 위를 두께만큼 안쪽으로 줄여 원래 색으로 덮음
					if (visual->outline && !IsLine(visual->shape)) {
						renderer.Draw(visual->shape, drawn, visual->outline->color, visual->rotation);
						drawn = Inset(visual->shape, drawn, visual->outline->width);
					}
					renderer.Draw(visual->shape, drawn, visual->color, visual->rotation);
				}
		}

	private:
		// 모든 변이 width만큼 평행하게 안으로 들어온 같은 모양
		// 삼각형은 중심이 옮겨지는데 회전은 각자 중심 기준이라, 회전한 삼각형의 테두리는 안쪽이 살짝 어긋남
		static Transform Inset(Shape shape, const Transform& transform, float width)
		{
			// 사각형은 변이 축과 나란해서 크기만 줄이면 됨
			if (shape == Shape::Rect)
				return { transform.pos, Max(transform.size - width * 2.f, Vec2{}) };

			// 삼각형: 내접원 중심 I는 세 변까지 거리가 같음(반지름 r)
			// → I 기준으로 k배 줄이면 세 변이 모두 r(1-k)만큼 들어옴. r(1-k) = width
			auto unit = UnitPolygon(shape);
			const Vec2 a = unit[0] * transform.size + transform.pos;
			const Vec2 b = unit[1] * transform.size + transform.pos;
			const Vec2 c = unit[2] * transform.size + transform.pos;
			const float la = Length(b - c), lb = Length(c - a), lc = Length(a - b);	// 각 꼭짓점 맞은편 변
			const float perimeter = la + lb + lc;
			if (perimeter <= 0.f)
				return transform;

			const Vec2 incenter = (a * la + b * lb + c * lc) / perimeter;
			const Vec2 ab = b - a, ac = c - a;
			const float area = std::abs(ab.x * ac.y - ab.y * ac.x) / 2.f;
			const float radius = area / (perimeter / 2.f);

			const float k = radius > 0.f ? std::max((radius - width) / radius, 0.f) : 0.f;
			return { incenter + (transform.pos - incenter) * k, transform.size * k };
		}
	};
}
