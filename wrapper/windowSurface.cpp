#include "windowSurface.h"

namespace LZ::Wrapper {


	WindowSurface::WindowSurface(Instance::Ptr instance, LZ::Core::Window::Ptr window) {

		mInstance = instance;

		if (glfwCreateWindowSurface(instance->getInstance(), window->getWindow(), nullptr, &mSurface) != VK_SUCCESS) {

			throw std::runtime_error("Error: failed to create surface");
		}
	}


	WindowSurface::~WindowSurface() {

		vkDestroySurfaceKHR(mInstance->getInstance(), mSurface, nullptr);
		mInstance.reset();
	}

}