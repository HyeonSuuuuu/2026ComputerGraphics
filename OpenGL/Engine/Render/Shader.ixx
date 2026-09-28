module;
#include <GL/glew.h>

export module hs.shader;

import std;
import hs.text;
export import hs.vec2;
export import hs.color;

export namespace hs
{
	class Shader
	{
	public:
		Shader(std::string_view vertexSource, std::string_view fragmentSource)
		{
			GLuint vertex = Compile(GL_VERTEX_SHADER, vertexSource);
			GLuint fragment = 0;
			try {
				fragment = Compile(GL_FRAGMENT_SHADER, fragmentSource);
			}
			catch (...) {
				glDeleteShader(vertex);
				throw;
			}

			_program = glCreateProgram();
			glAttachShader(_program, vertex);
			glAttachShader(_program, fragment);
			glLinkProgram(_program);
			glDeleteShader(vertex);
			glDeleteShader(fragment);

			GLint linked;
			glGetProgramiv(_program, GL_LINK_STATUS, &linked);
			if (!linked) {
				std::string log = ProgramLog(_program);
				glDeleteProgram(_program);
				throw std::runtime_error(Format("셰이더 링크 실패:\n{}", log));
			}
		}

		~Shader() { glDeleteProgram(_program); }

		Shader(Shader&& other) noexcept : _program(std::exchange(other._program, 0)) {}
		Shader& operator=(Shader&& other) noexcept
		{
			std::swap(_program, other._program);
			return *this;
		}

		void Use() const { glUseProgram(_program); }

		// 이름 검색이라 느림 → 생성할 때 받아 두고 Set에 넘길 것. 셰이더가 안 쓰는 uniform은 -1
		GLint UniformLocation(const char* name) const { return glGetUniformLocation(_program, name); }

		// Use()로 묶여 있는 프로그램에 적용됨
		static void Set(GLint location, float value) { glUniform1f(location, value); }
		static void Set(GLint location, Vec2 value) { glUniform2f(location, value.x, value.y); }
		static void Set(GLint location, Color value) { glUniform3f(location, value.r, value.g, value.b); }

	private:
		static GLuint Compile(GLenum type, std::string_view source)
		{
			GLuint shader = glCreateShader(type);
			const char* text = source.data();
			GLint length = static_cast<GLint>(source.size());
			glShaderSource(shader, 1, &text, &length);
			glCompileShader(shader);

			GLint compiled;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
			if (!compiled) {
				GLint size;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
				std::string log(size, '\0');
				glGetShaderInfoLog(shader, size, nullptr, log.data());
				glDeleteShader(shader);
				throw std::runtime_error(Format("{} 셰이더 컴파일 실패:\n{}",
					type == GL_VERTEX_SHADER ? "정점" : "프래그먼트", log));
			}
			return shader;
		}

		static std::string ProgramLog(GLuint program)
		{
			GLint size;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &size);
			std::string log(size, '\0');
			glGetProgramInfoLog(program, size, nullptr, log.data());
			return log;
		}

		GLuint _program{};
	};
}
