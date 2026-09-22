# OpenGL 엔진 (hs)

컴퓨터그래픽스 과제를 하면서 게임까지 키우는 것이 목표. 2D로 시작해 3D와 UI 분리까지 갈 계획.
**판단 기준 1순위는 가독성과 유지보수성.** SOLID는 목적이 아니라 그 수단.

## 빌드와 실행

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Enterprise\MSBuild\Current\Bin\MSBuild.exe" `
  "C:\Research\ComputerGraphics\OpenGL\OpenGL\OpenGL.vcxproj" `
  /p:Configuration=Debug /p:Platform=x64 /v:minimal /nologo
```

실행 파일은 `OpenGL/x64/Debug/OpenGL.exe`. 빌드 전에 실행 중인 프로세스를 먼저 종료할 것.
링커의 `LNK4098`(MSVCRT 충돌)은 vcpkg glfw3의 CRT 불일치로, 기존 경고라 무시해도 된다.

## 구조

```
Engine/Core       Check(단언), Component(IComponent), Random
Engine/Math       Shape(Vec2·Transform·Bounds·Color), Easing
Engine/Scene      Object(컴포넌트 컨테이너), Scene(소유·조회)
Engine/Movement   Mover, IMovementMode(ZigZag·Follow·EdgePatrol), MovementSystem
Engine/Animation  IAnimation·Animator·Playback, Animations(ScalePulse·ColorCycle), AnimationSystem
Engine/Collision  CollisionSystem (지금은 경계만, 오브젝트 충돌도 여기에)
Engine/Render     IRenderer, FirstRenderer(고정 파이프라인), Visual, RenderSystem
Engine/Platform   App(창·루프·입력), Input
```

프레임 순서는 앱의 `Update`에 그대로 보인다:
`AnimationSystem → MovementSystem → CollisionSystem → Scene::Flush`

## 설계 규칙

- **데이터와 실행을 나눈다.** 컴포넌트는 데이터, 시스템이 돌린다. `Object`에 로직을 넣지 않는다.
- **Movement는 위치와 속도**를 다룬다(충돌이 이 값들을 바꾼다). **Animation은 충돌이 안 건드리는 값**(크기·색·속력)을 시간 함수로 정한다.
- **종류는 상속이 아니라 구성으로.** `Object`를 상속하지 않는다. 종류가 다르면 붙이는 컴포넌트가 다른 것이고, 조립은 게임 쪽 함수가 한다(`Rect::Spawn`).
- **상속은 "끼우는 자리"에만** — `App` 계층, `IMovementMode`, `IAnimation`, `IRenderer`, `IComponent`.
- **컴포넌트는 게임 쪽에서 정의한다.** `struct Health : IComponent` 하면 끝이고 엔진은 안 바뀐다(예: `Subject/04/Rect.ixx`의 `Home`).
- **태그로 분기하지 않는다.** 처리가 다르면 다형성으로. 태그는 "적인가?" 묻는 용도로만.
- **추상화는 변화가 실제로 예상되는 경계부터.** 구현이 하나뿐인 인터페이스는 만들지 않는다.
- **안 쓰는 기능은 만들지 않는다.** 트리거가 왔을 때 만든다. 미사용 코드는 틀렸는지 알 수 없다.

## 주의할 것

- **순회 중 Spawn/Destroy는 예약된다.** `Scene::Flush()`가 프레임 끝에 반영한다. 직접 벡터를 건드리지 말 것.
- **`Get<T>()`는 정확히 같은 타입만 찾는다.** 컴포넌트를 상속하면 조용히 못 찾는다.
- `switch`에 `break` 빠뜨리면 MSVC가 경고하지 않는다. 일부러 흘릴 때는 `[[fallthrough]];`.
- 파일은 사용자가 동시에 편집하는 경우가 있다. **수정 전에 현재 내용을 확인할 것.**

## 코드 스타일

- 탭 들여쓰기, 한글 주석, 멤버는 `_소문자`.
- **코드가 말하는 것은 주석으로 쓰지 않는다.** 남길 것은 코드에 안 보이는 것뿐 — 왜 이 값인지, 불변식, 수명 주의, 겪었던 버그의 이유. 한 줄이면 충분하다.
- 단언은 `Check(조건, "무엇이 왜 틀렸는지")`. 계산이 비싼 검사는 `CheckSlow(람다, ...)`.
- 모듈 하나당 개념 하나. 인터페이스에 등장하는 타입은 `export import`로 함께 내보낸다.
