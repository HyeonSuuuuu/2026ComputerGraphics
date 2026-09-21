export module hs.object;

import hs.shape;
import hs.component;
import hs.movement;
import hs.visual;

export namespace hs
{
	// 화면에 존재하는 것 하나. 데이터를 들고 있고, 기능은 컴포넌트로 붙인다
	// 동작 자체는 컴포넌트와 시스템이 맡는다. 여기에 로직을 넣지 말 것
	class Object
	{
	public:
		explicit Object(const Transform& transform) : transform(transform) {}

		Transform transform;

		Component<Visual> visual;
		Component<Mover> mover;

		Bounds GetBounds() const { return transform.GetBounds(); }
	};
}
