#pragma once

#include "buffer.h"
#include "../texture/texture.h"
#include "../texture/cubeMapTexture.h"

namespace FF::Wrapper {

	struct UniformParameter {

		using Ptr = std::shared_ptr<UniformParameter>;
		static Ptr create() { return std::make_shared< UniformParameter>(); }

		size_t					mSize{ 0 };
		uint32_t				mBinding{ 0 };

		uint32_t				mCount{ 0 };
		VkDescriptorType		mDescriptorType;
		VkShaderStageFlags      mStage;

		std::vector<Buffer::Ptr> mBuffers{};
		Texture::Ptr mTexture{ nullptr };
		std::vector<Texture::Ptr> mTextures{};
		CubeMapTexture::Ptr mCubeMap{ nullptr };

		std::vector<VkDescriptorImageInfo> mImageInfos{};
	};

}
