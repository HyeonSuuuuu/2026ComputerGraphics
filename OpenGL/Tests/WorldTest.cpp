// World(ECS) 동작 고정용. World 속을 바꾼 뒤 이게 전부 O면 겉보기 동작은 그대로
// std::println 대신 cout: GCC 모듈에서 println 내부 정적 데이터가 Check.ixx와 겹쳐 링크 충돌

import std;
import hs.world;
import engine_test.components;

using namespace hs;
using namespace engine_test;

int failed = 0;

void Expect(bool ok, std::string_view what)
{
	if (!ok)
		++failed;
	std::cout << "  " << (ok ? "O" : "X") << ' ' << what << '\n';
}

int main()
{
	World world;

	std::cout << "[생성과 Flush]\n";
	Entity a = SpawnWithHome(world, 1.f);
	Expect(a.Get<Home>() && a.Get<Home>()->pos.y == 1.f, "다른 모듈에서 Add한 컴포넌트를 Get");
	Expect(!world.Find(a.Id()), "Flush 전에는 Find 안 됨");
	Expect(world.Size() == 0, "Flush 전 Size 0");
	world.Flush();
	Expect(world.Find(a.Id()) == a && world.Size() == 1, "Flush 후 보임");

	Entity b = SpawnWithHome(world, 2.f);
	b.Add<Speed>(2.f);
	b.Add<Owned>(std::make_unique<int>(20));
	Entity c = SpawnWithHome(world, 3.f);
	c.Add<Speed>(3.f);
	c.Add<Owned>(std::make_unique<int>(30));
	Entity d = SpawnWithHome(world, 4.f);
	d.Add<Speed>(4.f);
	d.Add<Owned>(std::make_unique<int>(40));
	world.Flush();

	std::cout << "[Each]\n";
	int visited = 0;
	float sum = 0.f;
	world.Each<Speed, Transform>([&](Entity, Speed& speed, Transform& transform)
		{
			++visited;
			sum += speed.value + transform.pos.x;
		});
	Expect(visited == 3 && sum == 18.f, "Each<Speed, Transform>: 둘 다 가진 3개만");

	std::cout << "[삭제]\n";
	world.Destroy(b);
	Expect(!world.Find(b.Id()) && !b.IsAlive(), "Destroy 즉시 목록에서 빠짐");
	world.Flush();
	Expect(!b && !world.Find(b.Id()), "Flush 후 옛 손잡이와 ID 무효");
	Expect(c.Get<Speed>()->value == 3.f && d.Get<Speed>()->value == 4.f, "중간 삭제 후 나머지 값 보존");
	Expect(*c.Get<Owned>()->value == 30 && *d.Get<Owned>()->value == 40, "이동 전용 컴포넌트도 보존");
	Expect(c.GetTransform().pos.x == 3.f && d.GetTransform().pos.x == 4.f, "Transform 보존");

	std::cout << "[칸 재사용]\n";
	Entity e = SpawnWithHome(world, 5.f);
	Expect(e.Id().index == b.Id().index && e.Id().generation != b.Id().generation, "칸 재사용 시 세대 증가");
	Expect(!e.Get<Speed>() && !e.Get<Owned>(), "재사용 칸에 옛 컴포넌트 없음");
	world.Flush();

	std::cout << "[조회]\n";
	std::vector<std::uint32_t> order;
	for (Entity entity : world.Entities())
		order.push_back(entity.Id().index);
	Expect(std::ranges::is_sorted(order) && order.size() == 4, "Entities()는 칸 순서");
	Expect(world.HitTest({ 3.f, 0.f }) == c && !world.HitTest({ 9.f, 9.f }), "HitTest");

	std::cout << "[Remove와 태그]\n";
	Expect(e.Remove<Home>() && !e.Get<Home>() && !e.Remove<Home>(), "Remove");
	e.Add<Tag>();
	Expect(e.Get<Tag>() != nullptr && !c.Get<Tag>(), "빈 태그 컴포넌트");

	std::cout << "[같은 프레임에 생성·삭제, 두 번 삭제]\n";
	std::size_t before = world.Size();
	Entity temp = SpawnWithHome(world, 7.f);
	temp.Add<Speed>(7.f);
	world.Destroy(temp);
	world.Destroy(temp);
	world.Flush();
	Expect(world.Size() == before && !temp, "만들자마자 지우면 한 번도 안 보이고 사라짐");
	Entity reused = SpawnWithHome(world, 8.f);
	Expect(reused.Id().index == temp.Id().index && !reused.Get<Speed>(), "그 칸은 한 번만 반납되고 옛 컴포넌트 없음");
	Entity other = SpawnWithHome(world, 9.f);
	Expect(other.Id().index != reused.Id().index, "두 번 Destroy해도 칸이 중복 반납되지 않음");
	world.Flush();

	std::cout << "[Clear]\n";
	world.Clear();
	Expect(!c && world.Size() == 0 && !world.Find(d.Id()), "Clear 후 전부 무효");

	std::cout << '\n' << (failed == 0 ? "전부 통과" : "실패 있음") << '\n';
	return failed;
}
