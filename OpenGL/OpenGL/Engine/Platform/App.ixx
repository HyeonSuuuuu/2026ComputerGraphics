module;
#include <gl/glew.h>
#include <gl/glfw3.h>

export module hs.app;

import std;
import hs.shape;
import hs.input;
import hs.renderer;
import hs.first_renderer;

export namespace hs
{
	class App
	{
	public:
		App(int width, int height, const char* title,
			RendererFactory makeRenderer = [] { return std::make_unique<FirstRenderer>(); })
		{
			if (!glfwInit()) {
				throw std::runtime_error("GLFW 초기화 실패!");
			}

			//--- OpenGL 버전 설정 (예: 3.3 Core Profile)
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

			_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
			if (!_window) {
				glfwTerminate();
				throw std::runtime_error("윈도우 생성 실패!");
			}

			glfwMakeContextCurrent(_window);

			glewExperimental = GL_TRUE; // 최신 기능 사용
			if (glewInit() != GLEW_OK) {
				glfwDestroyWindow(_window);
				glfwTerminate();
				throw std::runtime_error("GLEW 초기화 실패!");
			}
			_renderer = makeRenderer();
			if (!_renderer) {
				glfwDestroyWindow(_window);
				glfwTerminate();
				throw std::runtime_error("렌더러 생성 실패!");
			}

			glViewport(0, 0, width, height);
			glfwSwapInterval(1);

			// 콜백함수에서 this 캡처가 불가능해서 창에 App 주소를 붙여둠
			glfwSetWindowUserPointer(_window, this);

			glfwSetKeyCallback(_window, [](GLFWwindow* w, int key, int, int action, int) {
				auto* app = Self(w);
				if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
					app->Close();
				app->_input.OnKey(key, action);
				});

			glfwSetMouseButtonCallback(_window, [](GLFWwindow* w, int button, int action, int) {
				Self(w)->_input.OnMouseButton(button, action);
				});

			glfwSetCursorPosCallback(_window, [](GLFWwindow* w, double x, double y) {
				auto* app = Self(w);
				app->_input.OnCursor(app->ToNdc(x, y));
				});

			glfwSetScrollCallback(_window, [](GLFWwindow* w, double, double yOffset) {
				Self(w)->_input.OnScroll(static_cast<float>(yOffset));
				});

			// 첫 프레임 MouseDelta가 튀지 않게 현재 커서 위치로 초기화
			double x, y;
			glfwGetCursorPos(_window, &x, &y);
			_input.Init(ToNdc(x, y));
		}

		virtual ~App()
		{
			_renderer.reset();
			glfwDestroyWindow(_window);
			glfwTerminate();
		}

		App(const App&) = delete;
		App& operator=(const App&) = delete;

		void Run()
		{
			double last = glfwGetTime();
			while (!glfwWindowShouldClose(_window)) {
				_input.NewFrame();
				glfwPollEvents();

				double now = glfwGetTime();
				// 창 드래그 중엔 프레임이 멈춘다. 쌓인 시간을 그대로 넘기면 한 프레임에 크게 튄다
				float dt = std::min(static_cast<float>(now - last), MaxDelta);
				last = now;

				Update(dt);

				Render();

				// 버퍼 교체
				glfwSwapBuffers(_window);
			}
		}

	protected:
		virtual void Update(float dt) {}
		virtual void Render() {}

		void Close() { glfwSetWindowShouldClose(_window, true); }
		GLFWwindow* GetWindow() const { return _window; }
		const Input& GetInput() const { return _input; }
		IRenderer& GetRenderer() { return *_renderer; }
	private:
		GLFWwindow* _window{};
		Input		_input;
		std::unique_ptr<IRenderer> _renderer;

		// 20fps 밑으로 떨어지면 느려질지언정 건너뛰지는 않는다
		static constexpr float MaxDelta = 0.05f;



		static App* Self(GLFWwindow* w)
		{
			return static_cast<App*>(glfwGetWindowUserPointer(w));
		}

		// 픽셀 -> OpenGL 좌표(-1, 1)
		Vec2 ToNdc(double x, double y) const
		{
			int width, height;
			glfwGetWindowSize(_window, &width, &height);
			return {
				static_cast<float>(x / width * 2.0 - 1.0),
				static_cast<float>(1.0 - y / height * 2.0),
			};
		}
	};
}
