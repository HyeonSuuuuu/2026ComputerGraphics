export module hs.visual;

import hs.shape;
import hs.component;

export namespace hs
{
	struct Visual : IComponent
	{
		explicit Visual(Color color = {}) : color(color) {}

		Color color;
		float scale{ 1.f };			// 게임 쪽 배율. 애니메이션 불가침
		float effectScale{ 1.f };	// 연출 전용. 끝나면 1
		// 둘 다 보이는 크기에만. 충돌·경계는 transform.size
	};
}
