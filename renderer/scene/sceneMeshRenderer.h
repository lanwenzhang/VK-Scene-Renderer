#pragma once

#include "../../common.h"
#include "../../wrapper/device.h"
#include "../../wrapper/buffer.h"
#include "../../wrapper/commandPool.h"
#include "../../wrapper/commandBuffer.h"
#include "../../loader/scene.h"
#include "../../loader/mesh.h"
#include "../texture/texture.h"

namespace LZ::Renderer {

    class SceneMeshRenderer {
    public:
        using Ptr = std::shared_ptr<SceneMeshRenderer>;
        static Ptr create(const LZ::Wrapper::Device::Ptr& device, const LZ::Wrapper::CommandPool::Ptr& commandPool, LZ::Loader::MeshData& meshData, const LZ::Loader::Scene& scene) {

            return std::make_shared<SceneMeshRenderer>(device, commandPool, meshData, scene);
        }

        SceneMeshRenderer(const LZ::Wrapper::Device::Ptr& device, const LZ::Wrapper::CommandPool::Ptr& commandPool, LZ::Loader::MeshData& meshData, const LZ::Loader::Scene& scene);
        ~SceneMeshRenderer();

        void loadTextures(const LZ::Wrapper::Device::Ptr& device, const LZ::Wrapper::CommandPool::Ptr& commandPool, LZ::Loader::MeshData& meshData);
        void draw(const LZ::Wrapper::CommandBuffer::Ptr& cmd);

        std::vector<VkVertexInputBindingDescription> SceneMeshRenderer::getVertexInputBindingDescriptions() {
            return { VkVertexInputBindingDescription{ 0, sizeof(float) * 5, VK_VERTEX_INPUT_RATE_VERTEX } };
        }

        std::vector<VkVertexInputAttributeDescription> SceneMeshRenderer::getAttributeDescriptions() {
            return {

                { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
                { 1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 3 }
            };
        }


        [[nodiscard]] auto getDiffuseTextures() const { return mDiffuseTextures; }
        [[nodiscard]] auto getEmissiveTextures() const { return mEmissiveTextures; }
        [[nodiscard]] auto getOcclusionTextures() const { return mOcclusionTextures; }

    private:

        LZ::Wrapper::Device::Ptr mDevice{ nullptr };
        LZ::Wrapper::CommandPool::Ptr mCommandPool{ nullptr };
        LZ::Wrapper::Buffer::Ptr mVertexBuffer{ nullptr };
        LZ::Wrapper::Buffer::Ptr mIndexBuffer{ nullptr };
        LZ::Wrapper::Buffer::Ptr mIndirectBuffer{ nullptr };

        LZ::Loader::MeshData mMeshData;
        std::vector<Texture::Ptr> mDiffuseTextures{};
        std::vector<Texture::Ptr> mEmissiveTextures{};
        std::vector<Texture::Ptr> mOcclusionTextures{};

        uint32_t mDrawCount{ 0 };
    };

}

