# 개발 환경 설치: MSYS2 + GCC 툴체인 + 라이브러리
# 실행: powershell -ExecutionPolicy Bypass -File setup.ps1

$msys = 'C:\msys64'
$bash = "$msys\usr\bin\bash.exe"

if (-not (Test-Path $bash)) {
	winget install --id MSYS2.MSYS2 -e --accept-source-agreements --accept-package-agreements
}

# 첫 업데이트는 MSYS2 런타임 교체로 도중에 끝남 → 두 번
& $bash -lc 'pacman -Syu --noconfirm'
& $bash -lc 'pacman -Syu --noconfirm'

$packages = 'gcc', 'cmake', 'ninja', 'gdb', 'glfw', 'glew', 'glm' | ForEach-Object { "mingw-w64-ucrt-x86_64-$_" }
& $bash -lc "pacman -S --needed --noconfirm $($packages -join ' ')"

Write-Host ''
& "$msys\ucrt64\bin\g++.exe" --version | Select-Object -First 1
& "$msys\ucrt64\bin\cmake.exe" --version | Select-Object -First 1
