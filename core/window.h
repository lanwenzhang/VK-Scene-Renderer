#pragma once

#include "../common.h"

namespace lzvk::core {
	
	class Application;
	
	class Window {
	public:

		Window(const int& width, const int& height);
		~Window();

		using Ptr = std::shared_ptr<Window>;
		static Ptr create(const int& width, const int& height) { return std::make_shared<Window>(width, height); }

		bool shouldClose();

		void pollEvents();
		void setApp(std::shared_ptr<Application> app) { mApp = app; }
		void processEvent();

		[[nodiscard]] auto getWindow() const { return mWindow; }

	public:
		bool mWindowResized{ false };
		std::weak_ptr<Application> mApp;

	private:
		int mWidth{ 0 };
		int mHeight{ 0 };
		GLFWwindow* mWindow{ NULL };
	};
}