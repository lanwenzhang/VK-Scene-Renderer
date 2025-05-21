#include "device.h"

namespace FF::Wrapper {

	Device::Device(Instance::Ptr instance, WindowSurface::Ptr surface) {

		mInstance = instance;
		mSurface = surface;
		pickPhysicalDevice();
		initQueueFamilies(mPhysicalDevice);
		createLogicalDevice();
	}

	Device::~Device() {

		vkDestroyDevice(mDevice, nullptr);
		mSurface.reset();
		mInstance.reset();
	}

	void Device::pickPhysicalDevice() {

		// 1 Get physical device
		// 1.1 Get the number of physical device
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(mInstance->getInstance(), &deviceCount, nullptr);


		if (deviceCount == 0) {

			throw std::runtime_error("Error: failed to enumeratePhysicalDevice");
		}

		// 1.2 Put physical devices into the container
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(mInstance->getInstance(), &deviceCount, devices.data());

		// 2 Rate physical device
		std::multimap<int, VkPhysicalDevice> candidates;
		for (const auto& device : devices) {

			int score = rateDevice(device);
			candidates.insert(std::make_pair(score, device));

		}

		if (candidates.rbegin()->first > 0 && isDeviceSuitable(candidates.rbegin()->second)) {

			mPhysicalDevice = candidates.rbegin()->second;
		}

		if (mPhysicalDevice == VK_NULL_HANDLE) {

			throw std::runtime_error("Error:failed to get physical device");
		}
	}

	int Device::rateDevice(VkPhysicalDevice device) {

		int score = 0;

		// 1 Get device type verison
		VkPhysicalDeviceProperties deviceProp;
		vkGetPhysicalDeviceProperties(device, &deviceProp);

		// 2 Get device features 
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		if (deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {

			score += 1000;

		}

		score += deviceProp.limits.maxImageDimension2D;

		if (!deviceFeatures.geometryShader) {

			return 0;
		}

		return score;
	}

	bool Device::isDeviceSuitable(VkPhysicalDevice device) {

		// 1 Get device type verison
		VkPhysicalDeviceProperties deviceProp;
		vkGetPhysicalDeviceProperties(device, &deviceProp);

		// 2 Get device features
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		return deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader && deviceFeatures.samplerAnisotropy;
	}

	void Device::initQueueFamilies(VkPhysicalDevice device) {

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {

			// 1 Find graphic queue
			if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
				mGraphicQueueFamily = i;
			}

			// 2 Find present queue
			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface->getSurface(), &presentSupport);

			if (presentSupport) {

				mPresentQueueFamily = i;
			}


			if (isQueueFamilyComplete()) {

				break;
			}

			++i;
		}
	}

	void Device::createLogicalDevice() {

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> queueFamilies = { mGraphicQueueFamily.value(), mPresentQueueFamily.value() };
		float queuePriority = 1.0;

		// 1 Fill in queue family create info
		for (uint32_t queueFamily : queueFamilies) {

			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}
	
		// 2 Fill in device create info
		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceRequiredExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceRequiredExtensions.data();


		// 3 Enable validation layer
		if (mInstance->getEnableValidationLayer()) {
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			deviceCreateInfo.enabledLayerCount = 0;
		}

		// 4 Create device
		if (vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice) != VK_SUCCESS) {

			throw std::runtime_error("Error:failed to create logical device");
		}

		// 5 Create queue
		vkGetDeviceQueue(mDevice, mGraphicQueueFamily.value(), 0, &mGraphicQueue);


		vkGetDeviceQueue(mDevice, mPresentQueueFamily.value(), 0, &mPresentQueue);
	}

	bool Device::isQueueFamilyComplete() {

		return mGraphicQueueFamily.has_value() && mPresentQueueFamily.has_value();

	}

	VkSampleCountFlagBits Device::getMaxUsableSampleCount() {
		
		VkPhysicalDeviceProperties props{};
		vkGetPhysicalDeviceProperties(mPhysicalDevice, &props);

		VkSampleCountFlags counts = std::min(
			props.limits.framebufferColorSampleCounts,
			props.limits.framebufferDepthSampleCounts
		);

		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}


}