#pragma once

#include "../../common.h"
#include "../../wrapper/device.h"
#include "../../wrapper/buffer.h"
#include "../../wrapper/command_pool.h"
#include "../../wrapper/command_buffer.h"
#include "../../loader/scene.h"
#include "../../loader/mesh.h"
#include "../texture/texture.h"

namespace lzvk::renderer {

    class SceneMeshRenderer {
    public:
        using Ptr = std::shared_ptr<SceneMeshRenderer>;
        static Ptr create(const lzvk::wrapper::Device::Ptr& device, const lzvk::wrapper::CommandPool::Ptr& commandPool, lzvk::loader::MeshData& meshData, const lzvk::loader::Scene& scene) {

            return std::make_shared<SceneMeshRenderer>(device, commandPool, meshData, scene);
        }

        SceneMeshRenderer(const lzvk::wrapper::Device::Ptr& device, const lzvk::wrapper::CommandPool::Ptr& commandPool, lzvk::loader::MeshData& meshData, const lzvk::loader::Scene& scene);
        ~SceneMeshRenderer();

        void loadTextures(const lzvk::wrapper::Device::Ptr& device, const lzvk::wrapper::CommandPool::Ptr& commandPool, lzvk::loader::MeshData& meshData);
        void draw(const lzvk::wrapper::CommandBuffer::Ptr& cmd);

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

        lzvk::wrapper::Device::Ptr mDevice{ nullptr };
        lzvk::wrapper::CommandPool::Ptr mCommandPool{ nullptr };
        lzvk::wrapper::Buffer::Ptr mVertexBuffer{ nullptr };
        lzvk::wrapper::Buffer::Ptr mIndexBuffer{ nullptr };
        lzvk::wrapper::Buffer::Ptr mIndirectBuffer{ nullptr };

        lzvk::loader::MeshData mMeshData;
        std::vector<Texture::Ptr> mDiffuseTextures{};
        std::vector<Texture::Ptr> mEmissiveTextures{};
        std::vector<Texture::Ptr> mOcclusionTextures{};

        uint32_t mDrawCount{ 0 };
    };

}

