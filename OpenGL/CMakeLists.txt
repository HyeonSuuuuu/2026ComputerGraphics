cmake_minimum_required(VERSION 3.30)

# import std 지원이 아직 실험 기능. UUID는 CMake 버전마다 바뀜 (4.4.3 기준)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
set(CMAKE_CXX_MODULE_STD ON)

project(OpenGL CXX)

# UUID 불일치 시 import std가 조용히 꺼지고 컴파일 단계에서야 엉뚱한 에러 → 여기서 차단
if(NOT 26 IN_LIST CMAKE_CXX_COMPILER_IMPORT_STD)
	message(FATAL_ERROR
		"import std 사용 불가 (CMake ${CMAKE_VERSION}, ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}).\n"
		"MSYS2의 CMake·g++인지 확인. 맞다면 CMake 버전이 바뀌어 위 UUID가 달라진 것:\n"
		"cmake.exe 안의 CMAKE_EXPERIMENTAL_CXX_IMPORT_STD 옆 UUID로 교체")
endif()

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# typeid, dynamic_cast 금지, 타입 구분은 hs.type_id로
string(APPEND CMAKE_CXX_FLAGS " -fno-rtti")
# 컨트랙트(pre·contract_assert): Debug는 위반 시 출력 후 중단, 그 외엔 검사 안 함 (Check와 같게)
string(APPEND CMAKE_CXX_FLAGS " -fcontracts")
string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fcontract-evaluation-semantic=enforce")
foreach(config RELEASE RELWITHDEBINFO MINSIZEREL)
	string(APPEND CMAKE_CXX_FLAGS_${config} " -fcontract-evaluation-semantic=ignore")
endforeach()

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

# 모듈 파일(.ixx)을 target에 등록. GCC는 .ixx 확장자를 모름
function(hs_add_modules target)
	file(GLOB_RECURSE modules CONFIGURE_DEPENDS ${ARGN})
	target_sources(${target} PUBLIC FILE_SET CXX_MODULES FILES ${modules})
	set_source_files_properties(${modules} PROPERTIES LANGUAGE CXX)
endfunction()

# MSYS2 PATH 밖(Rider·탐색기)에서도 실행되도록: 런타임은 정적 링크, 필요한 DLL은 exe 옆에 복사
cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH TOOLCHAIN_BIN)
function(hs_runnable target)
	target_compile_options(${target} PRIVATE -Wall -Wextra)
	target_link_options(${target} PRIVATE -static-libgcc -static-libstdc++)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_RUNTIME_DLLS:${target}> ${TOOLCHAIN_BIN}/libwinpthread-1.dll $<TARGET_FILE_DIR:${target}>
		COMMAND_EXPAND_LISTS
	)
endfunction()

# ── 엔진: 게임과 테스트가 함께 쓰는 라이브러리 ──────────────────────────────
add_library(Engine STATIC)
hs_add_modules(Engine Engine/*.ixx)
target_compile_options(Engine PRIVATE -Wall -Wextra PUBLIC -freflection)
target_link_libraries(Engine PUBLIC OpenGL::GL glfw GLEW::GLEW glm::glm stdc++exp)

# ── 게임 ───────────────────────────────────────────────────────────────────
add_executable(OpenGL main.cpp)
hs_add_modules(OpenGL Subject/*.ixx Template/*.ixx)
target_link_libraries(OpenGL PRIVATE Engine)
hs_runnable(OpenGL)

# ── 테스트: 실행해서 X가 하나라도 나오면 실패(종료 코드 = 실패 수) ─────────────
enable_testing()
add_executable(EngineTests)
file(GLOB test_sources CONFIGURE_DEPENDS Tests/*.cpp)
target_sources(EngineTests PRIVATE ${test_sources})
hs_add_modules(EngineTests Tests/*.ixx)
target_link_libraries(EngineTests PRIVATE Engine)
hs_runnable(EngineTests)
add_test(NAME EngineTests COMMAND EngineTests)
