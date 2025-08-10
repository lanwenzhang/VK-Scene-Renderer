#include "cube_map_texture.h"
#include "../../tools/bitmap.h"
#include "../../tools/tools.h"

#include <stb_image.h>
#include <ktx.h>
#include <ktxvulkan.h>

namespace lzvk::renderer {

	//CubeMapTexture::CubeMapTexture(const lzvk::wrapper::Device::Ptr& device, const lzvk::wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath) {

	//	mDevice = device;

	//	// 1 Load hdr image
	//	int width, height, channels;
	//	float* hdrData = stbi_loadf(imageFilePath.c_str(), &width, &height, &channels, 4);
	//	if (!hdrData) {

	//		throw std::runtime_error("Failed to load HDR image");
	//	}

	//	lzvk::tools::Bitmap hdrBitmap(width, height, 4, lzvk::tools::BitmapFormat::Float, hdrData);
	//	stbi_image_free(hdrData);

	//	// 2 Hdr to cube map
	//	lzvk::tools::Bitmap cubemapBitmap = lzvk::tools::Tools::convertEquirectangularToCubemapFaces(hdrBitmap);

	//	mWidth = cubemapBitmap.mWidth;
	//	mHeight = cubemapBitmap.mHeight;

	//	// 3 Create cube map image
	//	mCubeMapImage = lzvk::wrapper::Image::create(mDevice,
	//		cubemapBitmap.mWidth,
	//		cubemapBitmap.mHeight,
	//		VK_FORMAT_R32G32B32A32_SFLOAT,
	//		VK_IMAGE_TYPE_2D,
	//		VK_IMAGE_TILING_OPTIMAL,
	//		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	//		VK_SAMPLE_COUNT_1_BIT,
	//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	//		VK_IMAGE_ASPECT_COLOR_BIT,
	//		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
	//		6,
	//		VK_IMAGE_VIEW_TYPE_CUBE);

	//	VkImageSubresourceRange initRange{};
	//	initRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//	initRange.baseMipLevel = 0;
	//	initRange.levelCount = 1;
	//	initRange.baseArrayLayer = 0;
	//	initRange.layerCount = 6;


	//	mCubeMapImage->setImageLayout(
	//		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	//		VK_PIPELINE_STAGE_TRANSFER_BIT,
	//		initRange,
	//		commandPool
	//	);


	//	const int facePixels = cubemapBitmap.mWidth * cubemapBitmap.mHeight;
	//	const int bytesPerPixel = cubemapBitmap.mChannels * lzvk::tools::Bitmap::getBytesPerChannel(cubemapBitmap.mFormat);
	//	const int faceBytes = facePixels * bytesPerPixel;

	//	assert(6 * faceBytes <= cubemapBitmap.mData.size());

	//	for (int face = 0; face < 6; ++face) {
	//		void* faceData = cubemapBitmap.mData.data() + face * faceBytes;
	//		mCubeMapImage->fillImageData(faceBytes, faceData, commandPool, face);
	//	}

	//	mCubeMapImage->setImageLayout(
	//		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	//		VK_PIPELINE_STAGE_TRANSFER_BIT,
	//		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	//		initRange,
	//		commandPool
	//	);


	//	// Create sampler
	//	mSampler = lzvk::wrapper::Sampler::create(mDevice);
	//	mImageInfo.imageLayout = mCubeMapImage->getLayout();
	//	mImageInfo.imageView = mCubeMapImage->getImageView();
	//	mImageInfo.sampler = mSampler->getSampler();

	//}


	CubeMapTexture::CubeMapTexture(const lzvk::wrapper::Device::Ptr& device,
		const lzvk::wrapper::CommandPool::Ptr& commandPool,
		const std::string& ktxFilePath) {

		mDevice = device;

		// 1. load ktx
		ktxTexture* texture;
		KTX_error_code result = ktxTexture_CreateFromNamedFile(ktxFilePath.c_str(),
			KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture);
		if (result != KTX_SUCCESS) {
			throw std::runtime_error("Failed to load .ktx cubemap: " + ktxFilePath);
		}

		// 2. check if it is cube map
		if (!(texture->numDimensions == 2 && texture->isCubemap)) {
			ktxTexture_Destroy(texture);
			throw std::runtime_error("Provided .ktx is not a cubemap: " + ktxFilePath);
		}

		// 3. Create vulkan image
		ktxVulkanDeviceInfo vkdi;
		result = ktxVulkanDeviceInfo_Construct(&vkdi, device->getPhysicalDevice(), device->getDevice(), device->getGraphicQueue(), commandPool->getCommandPool(), nullptr);
		if (result != KTX_SUCCESS) {
			ktxTexture_Destroy(texture);
			throw std::runtime_error("Failed to construct VulkanDeviceInfo for KTX");
		}

		ktxVulkanTexture vkTex;
		result = ktxTexture_VkUploadEx(texture, &vkdi, &vkTex, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		if (result != KTX_SUCCESS) {
			ktxVulkanDeviceInfo_Destruct(&vkdi);
			ktxTexture_Destroy(texture);
			throw std::runtime_error("Failed to upload KTX texture to Vulkan");
		}

		// 4. create cubemap imageView
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = vkTex.image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.format = vkTex.imageFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = texture->numLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 6;
		viewInfo.components = {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};

		VkImageView cubeView = VK_NULL_HANDLE;
		if (vkCreateImageView(device->getDevice(), &viewInfo, nullptr, &cubeView) != VK_SUCCESS) {
			ktxVulkanDeviceInfo_Destruct(&vkdi);
			ktxTexture_Destroy(texture);
			throw std::runtime_error("Failed to create cubemap VkImageView");
		}

		// 5. create image
		mCubeMapImage = lzvk::wrapper::Image::create(
			device,
			vkTex.image,
			cubeView,
			vkTex.imageFormat
		);

		// 6. set size
		mWidth = texture->baseWidth;
		mHeight = texture->baseHeight;

		// 7. create sampler and info
		mSampler = lzvk::wrapper::Sampler::create(mDevice);
		mImageInfo.imageLayout = mCubeMapImage->getLayout();
		mImageInfo.imageView = mCubeMapImage->getImageView();
		mImageInfo.sampler = mSampler->getSampler();

		// 8. clean up
		ktxTexture_Destroy(texture);
		ktxVulkanDeviceInfo_Destruct(&vkdi);
	}


	CubeMapTexture::CubeMapTexture(const lzvk::wrapper::Device::Ptr& device,
		const lzvk::wrapper::Image::Ptr& image)
	{
		mDevice = device;
		mCubeMapImage = image;

		mSampler = lzvk::wrapper::Sampler::create(mDevice);
		mImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		mImageInfo.imageView = mCubeMapImage->getImageView();
		mImageInfo.sampler = mSampler->getSampler();
	}

	CubeMapTexture::~CubeMapTexture(){}
}



