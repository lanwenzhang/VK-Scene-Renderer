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


	struct AttachmentDesc {
		
		VkFormat format;
		VkImageLayout imageLayout;
		VkSampleCountFlagBits samples;
		VkAttachmentLoadOp loadOp;
		VkAttachmentStoreOp storeOp;

		// color clear value
		std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

		// depth stencil clear
		float clearDepth = 1.0f;
		uint32_t clearStencil = 0;
	};

}
