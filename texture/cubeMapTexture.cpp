#include "cubeMapTexture.h"
#include"../stb_image.h"
#include "../tools/bitmap.h"
#include "../tools/tools.h"

namespace FF {

	CubeMapTexture::CubeMapTexture(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath) {

		mDevice = device;

		// 1 Load hdr image
		int width, height, channels;
		float* hdrData = stbi_loadf(imageFilePath.c_str(), &width, &height, &channels, 4);
		if (!hdrData) {

			throw std::runtime_error("Failed to load HDR image");
		}

		Bitmap hdrBitmap(width, height, 4, BitmapFormat::Float, hdrData);
		stbi_image_free(hdrData);

		// 2 Hdr to cube map
		Bitmap cubemapBitmap = Tools::convertEquirectangularToCubemapFaces(hdrBitmap);

		// 3 Create cube map image
		mCubeMapImage = Wrapper::Image::create(mDevice,
			cubemapBitmap.mWidth,
			cubemapBitmap.mHeight,
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
			6,
			VK_IMAGE_VIEW_TYPE_CUBE);

		VkImageSubresourceRange initRange{};
		initRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		initRange.baseMipLevel = 0;
		initRange.levelCount = 1;
		initRange.baseArrayLayer = 0;
		initRange.layerCount = 6;


		mCubeMapImage->setImageLayout(
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			initRange,
			commandPool
		);


		const int facePixels = cubemapBitmap.mWidth * cubemapBitmap.mHeight;
		const int bytesPerPixel = cubemapBitmap.mChannels * Bitmap::getBytesPerChannel(cubemapBitmap.mFormat);
		const int faceBytes = facePixels * bytesPerPixel;

		assert(6 * faceBytes <= cubemapBitmap.mData.size());

		for (int face = 0; face < 6; ++face) {
			void* faceData = cubemapBitmap.mData.data() + face * faceBytes;
			mCubeMapImage->fillImageData(faceBytes, faceData, commandPool, face);
		}

		mCubeMapImage->setImageLayout(
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			initRange,
			commandPool
		);


		// Create sampler
		mSampler = Wrapper::Sampler::create(mDevice);
		mImageInfo.imageLayout = mCubeMapImage->getLayout();
		mImageInfo.imageView = mCubeMapImage->getImageView();
		mImageInfo.sampler = mSampler->getSampler();

	}

	CubeMapTexture::~CubeMapTexture(){}
}



