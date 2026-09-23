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

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

file(GLOB_RECURSE MODULES CONFIGURE_DEPENDS
	Engine/*.ixx
	Subject/*.ixx
	Template/*.ixx
)

add_executable(OpenGL main.cpp)
target_sources(OpenGL PRIVATE FILE_SET CXX_MODULES FILES ${MODULES})

# GCC는 .ixx 확장자를 모름
set_source_files_properties(${MODULES} PROPERTIES LANGUAGE CXX)

target_compile_options(OpenGL PRIVATE -freflection -Wall -Wextra)
target_link_libraries(OpenGL PRIVATE OpenGL::GL glfw GLEW::GLEW glm::glm stdc++exp)

# MSYS2 PATH 밖(Rider·탐색기)에서도 실행되도록: 런타임은 정적 링크, glfw·glew DLL은 exe 옆에 복사
target_link_options(OpenGL PRIVATE -static-libgcc -static-libstdc++)
cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH TOOLCHAIN_BIN)
add_custom_command(TARGET OpenGL POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_RUNTIME_DLLS:OpenGL> ${TOOLCHAIN_BIN}/libwinpthread-1.dll $<TARGET_FILE_DIR:OpenGL>
	COMMAND_EXPAND_LISTS
)
