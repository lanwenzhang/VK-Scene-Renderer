#pragma once

#include "buffer.h"
#include "../renderer/texture/texture.h"
#include "../renderer/texture/cube_map_texture.h"

namespace lzvk::wrapper {

	struct UniformParameter {

		using Ptr = std::shared_ptr<UniformParameter>;
		static Ptr create() { return std::make_shared< UniformParameter>(); }

		size_t					mSize{ 0 };
		uint32_t				mBinding{ 0 };

		uint32_t				mCount{ 0 };
		VkDescriptorType		mDescriptorType;
		VkShaderStageFlags      mStage;

		std::vector<Buffer::Ptr> mBuffers{};
		lzvk::renderer::Texture::Ptr mTexture{ nullptr };
		lzvk::renderer::CubeMapTexture::Ptr mCubeMap{ nullptr };
		std::vector<lzvk::renderer::Texture::Ptr> mTextures{};

		std::vector<VkDescriptorImageInfo> mImageInfos{};
	};

}
