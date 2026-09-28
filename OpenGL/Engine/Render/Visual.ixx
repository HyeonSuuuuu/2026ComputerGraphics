export module hs.visual;

export import hs.color;
export import hs.shape;

import std;

export namespace hs
{
	// 도형 안쪽으로 width만큼 띠를 두름 → 바깥 크기·충돌은 그대로
	// 삼각형은 상자 중심으로 줄어서 변마다 두께가 조금 다름. 선은 면이 없어 테두리 없음
	struct Outline
	{
		Color color;
		float width{ 0.01f };
	};

	struct Visual
	{
		explicit Visual(Color color = {}) : color(color) {}

		Color color;
		Shape shape{ Shape::Rect };
		float rotation{};			// 라디안, 반시계. 보이는 것만 돌림 → 충돌 상자는 회전 전 그대로
		std::optional<Outline> outline;
		float scale{ 1.f };			// 게임 쪽 배율. 애니메이션 불가침
		float effectScale{ 1.f };	// 연출 전용. 끝나면 1
		// 둘 다 보이는 크기에만. 충돌·경계는 transform.size
	};
}
