# OpenGL 엔진 (hs)

컴퓨터그래픽스 과제를 하면서 게임까지 키우는 것이 목표. 2D로 시작해 3D와 UI 분리까지 갈 계획.
**판단 기준 1순위는 가독성과 유지보수성.** SOLID는 목적이 아니라 그 수단.

## 빌드와 실행

GCC 16 (MSYS2 UCRT64) + CMake + Ninja. Rider가 MinGW 툴체인으로 `cmake-build-debug/`에 빌드한다.

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
cmake --build cmake-build-debug
```

실행 파일은 `cmake-build-debug/OpenGL.exe` (glfw·glew DLL이 옆에 복사됨). 빌드 전에 실행 중인 프로세스를 먼저 종료할 것.
테스트는 `cmake-build-debug/EngineTests.exe` (종료 코드 = 실패 수). **엔진(특히 World) 속을 고치면 반드시 돌릴 것.**
라이브러리는 MSYS2 패키지: `mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,glfw,glew,glm}`.
C++26 + `-freflection` + `-fno-rtti`(`typeid`·`dynamic_cast` 불가) + `-fcontracts`(Debug는 위반 시 중단, 그 외 무시). `import std;`는 CMake 실험 기능이라 CMake를 올리면 `CMakeLists.txt`의 UUID도 바꿔야 한다.
MSVC 프로젝트(`.slnx`·`.vcxproj`)는 제거됨 — 필요하면 git 기록에서 복구.

## 구조

```
CMakeLists.txt    Engine(정적 라이브러리) + OpenGL(게임: Subject·Template) + EngineTests
main.cpp          실행할 과제 앱 선택
Subject/NN        과제별 앱과 게임 쪽 컴포넌트 (네임스페이스 appNN)
Template          새 과제 시작용 뼈대
Tests             EngineTests: World(ECS) 동작 고정. 컴포넌트를 다른 모듈에서 Add하는 경우 포함
Engine/Core       Check(단언), Random, TypeId(이름 해시·타입 순번), SparseSet
Engine/Math       Vec2(glm 래핑·Approach), Bounds(AABB), Easing
Engine/World      World(한 판 전체: 조립·조회·Each), Entity(번호표 손잡이), EntityTable(생존·세대·Flush),
                  ComponentPools(타입별 SparseSet 저장소), Transform(pos·size)
Engine/Movement   Mover, IMovementMode(ZigZag·Follow·EdgePatrol), MovementSystem
Engine/Effect     IEffect·EffectStack·Playback, Effects(ScalePulse·ColorCycle·ScaleIn), EffectSystem
Engine/Collision  CollisionSystem(경계·Separate·FindContacts, Trigger), ContactTracker(Enter/Stay/Exit)
Engine/Render     Color(HSV·RandomColor), IRenderer, FirstRenderer(고정 파이프라인), Visual, RenderSystem
Engine/Platform   App(창·루프·입력), Input
```

프레임 순서는 앱의 `Update`에 그대로 보인다:
`입력 처리(Spawn/Destroy) → World::Flush → (게임 로직) → EffectSystem → MovementSystem → CollisionSystem`
Flush가 시스템보다 앞: 이번 프레임에 만든 것도 바로 시스템 대상.

## 설계 규칙

- **데이터와 실행을 나눈다.** 컴포넌트는 데이터, 시스템이 돌린다. `Entity`에 로직을 넣지 않는다.
- **ECS.** 컴포넌트는 `World`의 타입별 `SparseSet`에 값으로 보관. `Entity`는 번호 + `World*`인 번호표라 값으로 넘긴다.
  시스템은 `world.Each<Mover, Transform>(람다)`로 필요한 저장소만 돈다. 그리기·충돌 쌍처럼 순서가 중요하면 `Entities()`(칸 순서).
- **Movement는 위치와 속도**를 다룬다(충돌이 이 값들을 바꾼다).
- **Effect는 `Visual`만 바꾼다.** 기준은 "꺼도 게임 결과가 같은가". `IEffect`가 `Visual&`만 받아 타입으로 강제된다.
  게임 결과가 달라지는 변화(먹으면 커짐 등)는 게임 로직이나 시스템이 한다(`App05::Grow` + `Approach`).
  `Animation`이라는 이름은 나중의 스프라이트·키프레임 애니메이션용으로 비워 둔다.
- **종류는 상속이 아니라 구성으로.** `Entity`를 상속하지 않는다. 종류가 다르면 붙이는 컴포넌트가 다른 것이고, 조립은 게임 쪽 함수가 한다(`Rect::Spawn`).
- **상속은 "끼우는 자리"에만** — `App` 계층, `IMovementMode`, `IEffect`, `IRenderer`.
- **컴포넌트는 게임 쪽에서 정의한다.** 평범한 `struct Health { ... };`면 끝이고 엔진은 안 바뀐다(예: `Subject/04/Rect.ixx`의 `Home`). 상속 불필요, 이동 가능하기만 하면 됨.
- **게임 코드는 `hs` 밖에.** 과제마다 `appNN` 네임스페이스, 파일 위에 `using namespace hs;`.
- **의존 방향은 Core·Math → World·Render → 시스템 → 게임.** 아래층이 위층을 import하지 않는다.
- **태그로 분기하지 않는다.** 처리가 다르면 다형성으로. 태그는 "적인가?" 묻는 용도로만.
- **추상화는 변화가 실제로 예상되는 경계부터.** 구현이 하나뿐인 인터페이스는 만들지 않는다.
- **안 쓰는 기능은 만들지 않는다.** 트리거가 왔을 때 만든다. 미사용 코드는 틀렸는지 알 수 없다.
  예외: 엔진의 범용 도구(Easing 함수들, Input의 Held/Released, World::HitTest 등)는 게임에서 곧 쓸 재사용 부품이라 유지.

## 주의할 것

- **순회 중 Spawn/Destroy는 예약된다.** 다음 `World::Flush()`에서 반영된다. 직접 벡터를 건드리지 말 것.
- **`Get`으로 받은 포인터·참조는 그 자리에서만.** 같은 타입의 Add·삭제가 저장소를 재배치(삭제는 끝 원소로 구멍을 메움). 프레임을 넘기면 `EntityId`.
- **`Each` 안에서 Add·Remove·Spawn 금지.** `Check`로 막혀 있다. 모아 뒀다가 순회 뒤에.
- **`Get<T>()`는 정확히 같은 타입만 찾는다.** 컴포넌트를 상속하면 조용히 못 찾는다.
- **`TypeIdOf<T>`는 이름(`hs::Visual`)의 해시.** 과제마다 같은 이름으로 컴포넌트를 만들면 ID가 겹친다 → 과제별 네임스페이스로.
  이름은 리플렉션으로 타입을 분해해 조립(`Tween<const app04::Home*>`까지). 통째로 `display_string_of`를 쓰면 안 됨:
  GCC가 import한 쪽에서만 `@모듈`을 붙여, 정의한 모듈과 쓰는 모듈의 ID가 달라진다(`Get`이 조용히 nullptr).
  배열·함수 타입·volatile·enum 값 인자·익명 네임스페이스 타입은 컴파일 에러 → 필요해지면 `Name()`에 규칙 추가.
- `switch`에서 일부러 흘릴 때는 `[[fallthrough]];`.
- **모듈 간 전방 선언 불가.** `friend class X;`는 X를 선언한 모듈 소속으로 만든다 → 같은 모듈이어야 함. `Entity`와 `World`가 한 파일인 이유이기도 함.
- **모듈 인터페이스의 `inline` 함수 안 `static` 금지.** GCC가 import한 파일마다 따로 만들어 링크 충돌(`NextTypeIndex` 주석). `inline` 없이 정의하면 한 곳에만 생긴다.
- **glm은 `hs.vec2`에서만 include.** 다른 모듈에서 include하면 `GLM_FORCE_CTOR_INIT`가 빠져 `Vec2` 정의가 둘이 됨(ODR). 필요한 glm 함수는 `hs.vec2`에 감싸서 추가.
- **`auto` 반환 멤버 함수는 클래스 안에서 쓰는 곳보다 먼저 정의.** GCC는 본문을 순서대로 추론한다.
- **`pre`에서 GCC 내부 오류(ICE, gimplify.cc)가 나면 그 자리는 `Check`로.** `Effect.ixx`의 `Playback::Seek`가 그 경우(GCC 16.2).
- **import를 바꾼 뒤 `dependency cycle`이 나면** 소스가 아니라 ninja의 옛 기록일 수 있음 → `cmake-build-debug/.ninja_deps` 삭제 후 빌드.
- 파일은 사용자가 동시에 편집하는 경우가 있다. **수정 전에 현재 내용을 확인할 것.**

## 코드 스타일

- 탭 들여쓰기, 한글 주석, 멤버는 `_소문자`.
- **코드가 말하는 것은 주석으로 쓰지 않는다.** 남길 것은 코드에 안 보이는 것뿐 — 왜 이 값인지, 불변식, 수명 주의, 겪었던 버그의 이유. 한 줄이면 충분하다.
- 단언은 `Check(조건, "무엇이 왜 틀렸는지")`. 계산이 비싼 검사는 `CheckSlow(람다, ...)`.
- **함수 인자의 전제 조건이고 조건식만으로 이유가 보이면 `pre (조건)`** (C++26 컨트랙트). 위반 시 조건식이 그대로 출력된다(`Approach`, `SetBounds`, `FollowMode`).
  이유 설명이 필요하면 `Check`(메시지 인자가 있음). `pre`의 줄 번호는 부른 쪽이 아니라 `pre`를 적은 쪽.
- 모듈 하나당 개념 하나. 인터페이스에 등장하는 타입은 `export import`로 함께 내보낸다.
