module;
#include <GL/glew.h>
#include <GLFW/glfw3.h>

export module hs.app;

import std;
import hs.vec2;
import hs.text;
import hs.type_id;
export import hs.input;
export import hs.renderer;
import hs.first_renderer;

export namespace hs
{
	class App
	{
	public:
		App(int width, int height,
			RendererFactory makeRenderer = [] { return std::make_unique<FirstRenderer>(); })
		{
			if (!glfwInit()) {
				throw std::runtime_error("GLFW 초기화 실패!");
			}

			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

			_window = glfwCreateWindow(width, height, "", nullptr, nullptr);	// 이름은 Run에서
			if (!_window) {
				glfwTerminate();
				throw std::runtime_error("윈도우 생성 실패!");
			}

			glfwMakeContextCurrent(_window);

			glewExperimental = GL_TRUE;
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

			// 콜백에서 this 캡처 불가 → 창에 App 주소 보관
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

			// 첫 프레임 MouseDelta 튐 방지
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

		// deducing this: app.Run()을 부른 객체의 실제 타입(App04 등)이 Self → 리플렉션으로 창 이름
		// 생성자에선 파생 클래스가 아직 없어 알 수 없으므로 여기서
		template<class Self>
		void Run(this Self& self)
		{
			static_cast<App&>(self).Loop(ShortTypeName<std::remove_cv_t<Self>>);
		}

	private:
		void Loop(std::string_view name)
		{
			_name = name;
			RefreshTitle();

			double last = glfwGetTime();
			while (!glfwWindowShouldClose(_window)) {
				_input.NewFrame();
				glfwPollEvents();

				double now = glfwGetTime();
				// 창 드래그 중 프레임 정지 → 누적 시간 한 번에 튐 방지
				float dt = std::min(static_cast<float>(now - last), MaxDelta);
				last = now;

				Update(dt);

				Render();

				glfwSwapBuffers(_window);
			}
		}

	protected:
		virtual void Update(float) {}
		virtual void Render() {}

		void Close() { glfwSetWindowShouldClose(_window, true); }
		// 창 제목 = "이름 - 상태". 운영체제 호출이라 상태가 바뀔 때만 부를 것
		void SetStatus(std::string_view status)
		{
			_status = status;
			RefreshTitle();
		}
		GLFWwindow* GetWindow() const { return _window; }
		const Input& GetInput() const { return _input; }
		IRenderer& GetRenderer() { return *_renderer; }
	private:
		// 생성자에서 SetStatus를 불러도 이름은 Run에서야 정해짐 → 둘을 따로 들고 매번 합침
		void RefreshTitle()
		{
			std::string title = _status.empty() ? _name : Format("{} - {}", _name, _status);
			glfwSetWindowTitle(_window, title.c_str());
		}

		std::string _name;
		std::string _status;
		GLFWwindow* _window{};
		Input		_input;
		std::unique_ptr<IRenderer> _renderer;

		// 20fps 미만은 건너뛰기 대신 감속
		static constexpr float MaxDelta = 0.05f;



		static App* Self(GLFWwindow* w)
		{
			return static_cast<App*>(glfwGetWindowUserPointer(w));
		}

		// 픽셀 → NDC(-1~1)
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
