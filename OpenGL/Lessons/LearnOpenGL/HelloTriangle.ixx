module;
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// learnopengl.com — Getting started / Hello Triangle
// 1: 삼각형(VBO+VAO)   2: 사각형(EBO)   W: 와이어프레임
export module lesson.hello_triangle;

import std;
import hs;

using namespace hs;

namespace lesson
{
	const char* VertexShaderSource = R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;
		void main()
		{
			gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
		}
	)";

	const char* FragmentShaderSource = R"(
		#version 330 core
		out vec4 FragColor;
		void main()
		{
			FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
		}
	)";
	
	const char* FragmentShader2Source = R"(
		#version 330 core
		out vec4 FragColor;
		void main()
		{
			FragColor = vec4(1.f, 1.f, 0.f, 1.f);
		}
	)";

	GLuint CompileShader(GLenum type, const char* source)
	{
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);

		int success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			std::cout << Format("셰이더 컴파일 실패:\n{}\n", infoLog);
		}
		return shader;
	}
	
	void LinkProgram(GLuint program)
	{
		glLinkProgram(program);
			
		int success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(program, 512, nullptr, infoLog);
			std::cout << Format("셰이더 링크 실패:\n{}\n", infoLog);
		}
	}
}

export namespace lesson
{
	class HelloTriangle : public App
	{
		using Super = App;
	public:
		HelloTriangle(int width, int height)
			: Super(width, height)
		{
			// ── 셰이더 프로그램 ──
			GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, VertexShaderSource);
			GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, FragmentShaderSource);
			GLuint fragmentShaderYellow = CompileShader(GL_FRAGMENT_SHADER, FragmentShader2Source);
			
			_shaderProgram = glCreateProgram();
			glAttachShader(_shaderProgram, vertexShader);
			glAttachShader(_shaderProgram, fragmentShader);
			LinkProgram(_shaderProgram);
			
			_shaderYellowProgram = glCreateProgram();
			glAttachShader(_shaderYellowProgram, vertexShader);
			glAttachShader(_shaderYellowProgram, fragmentShaderYellow);
			LinkProgram(_shaderYellowProgram);
			
			// 프로그램에 링크된 뒤엔 필요 없음
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);
			glDeleteShader(fragmentShaderYellow);

			// ── 삼각형: VAO가 아래 VBO 연결과 속성 설정을 기억 ──
			float triangle[] = {
				-0.5f, -0.5f, 0.0f,
				 0.5f, -0.5f, 0.0f,
				 0.0f,  0.5f, 0.0f,
				// second triangle
				0.0f, -0.5f, 0.0f,  // left
				0.9f, -0.5f, 0.0f,  // right
				0.45f, 0.5f, 0.0f   // top 
			};
			
			float triangle2[]= {
				0.0f, -0.5f, 0.0f,  // left
				0.9f, -0.5f, 0.0f,  // right
				0.45f, 0.5f, 0.0f   // top 
			};
			
			glGenVertexArrays(1, &_vaos[0]);
			glGenBuffers(1, &_vbos[0]);

			glBindVertexArray(_vaos[0]);
			glBindBuffer(GL_ARRAY_BUFFER, _vbos[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
			glEnableVertexAttribArray(0);
			
			// Yellow
			glGenVertexArrays(1, &_vaos[1]);
			glGenBuffers(1, &_vbos[1]);

			glBindVertexArray(_vaos[1]);
			glBindBuffer(GL_ARRAY_BUFFER, _vbos[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(triangle2), triangle2, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
			glEnableVertexAttribArray(0);
			

			// ── 사각형: 정점 4개 + 인덱스 6개(삼각형 2개) ──
			float rectangle[] = {
				 0.5f,  0.5f, 0.0f,	// 오른쪽 위
				 0.5f, -0.5f, 0.0f,	// 오른쪽 아래
				-0.5f, -0.5f, 0.0f,	// 왼쪽 아래
				-0.5f,  0.5f, 0.0f,	// 왼쪽 위
			};
			unsigned int indices[] = {
				0, 1, 3,
				1, 2, 3,
			};
			glGenVertexArrays(1, &_rectVao);
			glGenBuffers(1, &_rectVbo);
			glGenBuffers(1, &_rectEbo);

			glBindVertexArray(_rectVao);
			glBindBuffer(GL_ARRAY_BUFFER, _rectVbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle), rectangle, GL_STATIC_DRAW);
			// EBO 바인딩은 VAO에 저장됨 → VAO가 묶인 동안 바인딩하고, VAO보다 먼저 풀면 안 됨
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rectEbo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
			glEnableVertexAttribArray(0);

			glBindVertexArray(0);
		}

		// App 소멸자가 컨텍스트를 없애기 전에 실행됨
		~HelloTriangle() override
		{
			glDeleteVertexArrays(2, _vaos);
			glDeleteBuffers(2, _vbos);
			glDeleteVertexArrays(1, &_rectVao);
			glDeleteBuffers(1, &_rectVbo);
			glDeleteBuffers(1, &_rectEbo);
			glDeleteProgram(_shaderProgram);
			glDeleteProgram(_shaderYellowProgram);
		}

	protected:
		void Update(float) override
		{
			const auto& input = GetInput();

			if (input.IsKeyPressed(GLFW_KEY_1))
			{
				_showRect = false;
				_showYellow = false;
			}
			if (input.IsKeyPressed(GLFW_KEY_2))
			{
				_showRect = true;
				_showYellow = false;
			}
			
			if (_showRect == false)
				if (input.IsKeyPressed(GLFW_KEY_3))
					_showYellow = true;
			if (input.IsKeyPressed(GLFW_KEY_W)) {
				_wireframe = !_wireframe;
				glPolygonMode(GL_FRONT_AND_BACK, _wireframe ? GL_LINE : GL_FILL);
			}
		}

		void Render() override
		{
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glUseProgram(_shaderProgram);
			if (_showRect) {
				glBindVertexArray(_rectVao);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			}
			else if (_showYellow)
			{
				glBindVertexArray(_vaos[0]);
				glDrawArrays(GL_TRIANGLES, 0, 3);
				glUseProgram(_shaderYellowProgram);
				glBindVertexArray(_vaos[1]);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}
			else {
				glBindVertexArray(_vaos[0]);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
			
			
		}

	private:
		GLuint _shaderProgram{}, _shaderYellowProgram{};
		GLuint _vaos[2], _vbos[2];
		GLuint _rectVao{}, _rectVbo{}, _rectEbo{};
		bool _showRect = false;
		bool _wireframe = false;
		bool _showYellow = false;
	};
}
