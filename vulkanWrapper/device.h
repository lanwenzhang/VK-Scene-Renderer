#pragma once

#include "../base.h"
#include "instance.h"
#include "windowSurface.h"


namespace FF::Wrapper {


	const std::vector<const char*> deviceRequiredExtensions = {

		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_MAINTENANCE1_EXTENSION_NAME // For flip y-axis
	};

	class Device {
	public:

		using Ptr = std::shared_ptr<Device>;
		static Ptr create(Instance::Ptr instance, WindowSurface::Ptr surface) { return std::make_shared<Device>(instance, surface); }

		Device(Instance::Ptr instance, WindowSurface::Ptr surface);
		~Device();

		// 1 Create Physical device and logical device
		void pickPhysicalDevice();

		// 1.1 Physical device properties
		int rateDevice(VkPhysicalDevice device);
		bool isDeviceSuitable(VkPhysicalDevice device);

		// 1.2 Physical device queues
		void initQueueFamilies(VkPhysicalDevice device);
		bool isQueueFamilyComplete();

		// 1.3 Logical device
		void createLogicalDevice();


		VkSampleCountFlagBits getMaxUsableSampleCount();
		// 2 For external call
		[[nodiscard]] auto getDevice() const { return mDevice;}
		[[nodiscard]] auto getPhysicalDevice() const { return mPhysicalDevice; }
		[[nodiscard]] auto getGraphicQueueFamily() const { return mGraphicQueueFamily; }
		[[nodiscard]] auto getPresentQueueFamily() const { return mPresentQueueFamily; }
		[[nodiscard]] auto getGraphicQueue() const { return mGraphicQueue;}
		[[nodiscard]] auto getPresentQueue() const { return mPresentQueue; }

	private:

		Instance::Ptr mInstance{ nullptr };
		VkDevice mDevice{ VK_NULL_HANDLE };
		VkPhysicalDevice mPhysicalDevice{ VK_NULL_HANDLE };
		WindowSurface::Ptr mSurface{ nullptr };

		// Graphics queue class id
		std::optional<uint32_t> mGraphicQueueFamily;
		VkQueue mGraphicQueue{ VK_NULL_HANDLE };

		// Present queue class id
		std::optional<uint32_t> mPresentQueueFamily;
		VkQueue mPresentQueue{ VK_NULL_HANDLE };

		// Anti-aliasing
		VkSampleCountFlagBits mSampleCounts{ VK_SAMPLE_COUNT_1_BIT };
	
	};

}