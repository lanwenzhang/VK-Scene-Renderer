#include "window.h"
#include "application.h"

namespace LZ::Core {

	static void windowResized(GLFWwindow* window, int width, int height) {

		auto pUserData = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		pUserData->mWindowResized = true;
	}

	static void cursorPosCallBack(GLFWwindow* window, double xpos, double ypos) {

		auto pUserData = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		auto app = pUserData->mApp;

		if (!app.expired()) {
			auto appReal = app.lock();
			appReal->onMouseMove(xpos, ypos);
		}
	}


	Window::Window(const int& width, const int& height) {

		mWidth = width;
		mHeight = height;
		// 1 Init glfw
		glfwInit();

		// 1.1 Disable opengl
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// 1.2 Disable the window size change
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		// 2 Create a window
		mWindow = glfwCreateWindow(mWidth, mHeight, "vulkan", nullptr, nullptr);
		if (!mWindow) {

			std::cerr << "Error: failed to create window" << std::endl;
		}

		// 3 Resize a window
		glfwSetWindowUserPointer(mWindow, this);
		glfwSetFramebufferSizeCallback(mWindow, windowResized);
		glfwSetCursorPosCallback(mWindow, cursorPosCallBack);
		glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	Window::~Window() {

		glfwDestroyWindow(mWindow);
		glfwTerminate();

	}

	bool Window::shouldClose() {

		return glfwWindowShouldClose(mWindow);
	}

	void Window::pollEvents() {

		glfwPollEvents();
	}

	void Window::processEvent() {
		
		if (mApp.expired()) {
			
			return;
		}

		auto app = mApp.lock();

		if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			
			exit(0);
		}

		if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS) {
			
			app->onKeyDown(CAMERA_MOVE::MOVE_FRONT);
		}

		if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS) {
			app->onKeyDown(CAMERA_MOVE::MOVE_BACK);
		}

		if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS) {
			app->onKeyDown(CAMERA_MOVE::MOVE_LEFT);
		}

		if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS) {
			app->onKeyDown(CAMERA_MOVE::MOVE_RIGHT);
		}

		if (glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			
			app->enableMouseControl(true);
		}
		else {
			app->enableMouseControl(false);
		}

	}
}