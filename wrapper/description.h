#pragma once

#include "buffer.h"
#include "../renderer/texture/texture.h"
#include "../renderer/texture/cubeMapTexture.h"

namespace LZ::Wrapper {

	struct UniformParameter {

		using Ptr = std::shared_ptr<UniformParameter>;
		static Ptr create() { return std::make_shared< UniformParameter>(); }

		size_t					mSize{ 0 };
		uint32_t				mBinding{ 0 };

		uint32_t				mCount{ 0 };
		VkDescriptorType		mDescriptorType;
		VkShaderStageFlags      mStage;

		std::vector<Buffer::Ptr> mBuffers{};
		LZ::Renderer::Texture::Ptr mTexture{ nullptr };
		LZ::Renderer::CubeMapTexture::Ptr mCubeMap{ nullptr };
		std::vector<LZ::Renderer::Texture::Ptr> mTextures{};

		std::vector<VkDescriptorImageInfo> mImageInfos{};
	};

}
