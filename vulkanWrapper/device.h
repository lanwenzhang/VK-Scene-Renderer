#pragma once

#include "../base.h"
#include "instance.h"
#include "windowSurface.h"


namespace FF::Wrapper {


	const std::vector<const char*> deviceRequiredExtensions = {

		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_MAINTENANCE1_EXTENSION_NAME,
		VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
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

		PFN_vkGetBufferDeviceAddress getBufferDeviceAddressFunction() const;

		// 2 For external call
		[[nodiscard]] auto getDevice() const { return mDevice;}
		[[nodiscard]] auto getPhysicalDevice() const { return mPhysicalDevice; }
		[[nodiscard]] auto getGraphicQueueFamily() const { return mGraphicQueueFamily; }
		[[nodiscard]] auto getPresentQueueFamily() const { return mPresentQueueFamily; }
		[[nodiscard]] auto getComputeQueueFamily() const { return mComputeQueueFamily; }
		[[nodiscard]] auto getGraphicQueue() const { return mGraphicQueue;}
		[[nodiscard]] auto getPresentQueue() const { return mPresentQueue; }
		[[nodiscard]] auto getComputeQueue() const { return mComputeQueue; }

	private:

		Instance::Ptr mInstance{ nullptr };
		WindowSurface::Ptr mSurface{ nullptr };
		VkPhysicalDevice mPhysicalDevice{ VK_NULL_HANDLE };
		VkDevice mDevice{ VK_NULL_HANDLE };

		std::optional<uint32_t> mGraphicQueueFamily;
		std::optional<uint32_t> mPresentQueueFamily;
		std::optional<uint32_t> mComputeQueueFamily;

		VkQueue mGraphicQueue{ VK_NULL_HANDLE };
		VkQueue mPresentQueue{ VK_NULL_HANDLE };
		VkQueue mComputeQueue{ VK_NULL_HANDLE };

		VkSampleCountFlagBits mSampleCounts{ VK_SAMPLE_COUNT_1_BIT };
		PFN_vkGetBufferDeviceAddress fpGetBufferDeviceAddress = nullptr;
	};

}