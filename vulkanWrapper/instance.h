#pragma once

#include "../base.h"

namespace FF::Wrapper {

	class Instance {
	public:

		// 1 Instance
		using Ptr = std::shared_ptr<Instance>;
		static Ptr create(bool enableValidationLayer) { return std::make_shared<Instance>(enableValidationLayer); }

		Instance(bool enableValidationLayer);
		~Instance();

		// Extensions
		void printAvailableExtensions();
		std::vector<const char*> getRequiredExtensions();

		// Layers
		bool checkValidationLayerSupport();

		// Debugger
		void setupDebugger();

		// 2 Outside call
		[[nodiscard]] VkInstance getInstance() const { return mInstance; }
		[[nodiscard]] bool getEnableValidationLayer() const { return mEnableValidationLayer; }

	private:

		VkInstance mInstance{ VK_NULL_HANDLE };
		bool mEnableValidationLayer{ false };
		VkDebugUtilsMessengerEXT mDebugger{ VK_NULL_HANDLE };
	};

}