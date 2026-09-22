export module hs.visual;

import hs.shape;
import hs.component;

export namespace hs
{
	// 어떻게 보이는지
	struct Visual : IComponent
	{
		explicit Visual(Color color = {}) : color(color) {}

		Color color;
		float scale{ 1.f };	// 보이는 크기 배율. 충돌·경계는 transform.size를 쓴다
	};
}
