# OpenGL 엔진

GCC 16 + CMake + Ninja (C++26, 모듈, `import std;`). Windows 전용.

## 처음 한 번: 설치

```powershell
powershell -ExecutionPolicy Bypass -File setup.ps1
```

MSYS2를 `C:\msys64`에 설치하고 GCC·CMake·Ninja·gdb·GLFW·GLEW·GLM을 받는다.
마지막에 `g++ 16.x`, `cmake 4.4.x`가 출력되면 성공.

## Rider 설정

**1. 툴체인** — Settings → Build, Execution, Deployment → Toolchains → `+` → MinGW

| 항목 | 값 |
|---|---|
| 도구 모음 | `C:\msys64\ucrt64` |
| CMake | `C:\msys64\ucrt64\bin\cmake.exe` ← **Rider 내장 CMake 말고 이것** |
| 빌드 도구 | `C:\msys64\ucrt64\bin\ninja.exe` |
| C 컴파일러 | `C:\msys64\ucrt64\bin\gcc.exe` |
| C++ 컴파일러 | `C:\msys64\ucrt64\bin\g++.exe` |
| 디버거 | `C:\msys64\ucrt64\bin\gdb.exe` |

MinGW를 목록 맨 위로 올려 기본값으로.

**2. 열기** — File → Open → 이 폴더의 `CMakeLists.txt` → Open as Project

**3. 프로필** — Settings → Build, Execution, Deployment → CMake에서 `debug` 프리셋(Debug (GCC))이 보이면 그걸 사용.
안 보이면 `+`로 Build type `Debug`, Toolchain `MinGW`, Generator `Ninja`.

**4. 실행** — 실행 대상 `OpenGL` 선택 후 실행.

## 명령줄 빌드

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
cmake --preset debug
cmake --build --preset debug
.\cmake-build-debug\OpenGL.exe
```

## 자주 나는 문제

| 증상 | 원인 |
|---|---|
| `import std 사용 불가` (CMake 설정 단계) | Rider 내장 CMake를 쓰는 중 → 툴체인의 CMake 경로 확인. MSYS2 CMake가 4.4가 아니면 `CMakeLists.txt`의 UUID 교체 필요 |
| `'meta': No such file` 또는 `cl.exe` 관련 에러 | Visual Studio 툴체인으로 빌드 중 → 프로필의 Toolchain을 MinGW로 |
| exe 실행 시 DLL 없음 | 빌드 폴더의 exe를 실행할 것 (glfw·glew DLL이 옆에 복사됨) |
