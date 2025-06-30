#pragma once

#include "../common.h"
#include "instance.h"
#include "../core/window.h"

namespace LZ::Wrapper {

	class WindowSurface {
	public:

		WindowSurface(Instance::Ptr instance, LZ::Core::Window::Ptr window);
		~WindowSurface();

		using Ptr = std::shared_ptr<WindowSurface>;
		static Ptr create(Instance::Ptr instance, LZ::Core::Window::Ptr window) { return std::make_shared<WindowSurface>(instance, window); }

		[[nodiscard]] auto getSurface() const { return mSurface; }

	private:

		VkSurfaceKHR mSurface{ VK_NULL_HANDLE };
		Instance::Ptr mInstance{ nullptr };

	};

}