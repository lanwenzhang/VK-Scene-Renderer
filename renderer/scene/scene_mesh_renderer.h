#pragma once

#include "../../common.h"
#include "../../wrapper/device.h"
#include "../../wrapper/buffer.h"
#include "../../wrapper/command_pool.h"
#include "../../wrapper/command_buffer.h"
#include "../../loader/scene.h"
#include "../../loader/mesh.h"
#include "../texture/texture.h"

#include "../../wrapper/descriptor_pool.h"
#include "../../wrapper/descriptor_set.h"
#include "../../wrapper/descriptor_set_layout.h"

#include "../uniform/transform_uniform_manager.h"
#include "../uniform/material_uniform_manager.h"
#include "../uniform/draw_data_uniform_manager.h"
#include "../uniform/scene_texture_manager.h"

namespace lzvk::renderer {

    class SceneMeshRenderer {
    public:
        using Ptr = std::shared_ptr<SceneMeshRenderer>;
        static Ptr create(const lzvk::wrapper::Device::Ptr& device, 
                          const lzvk::wrapper::CommandPool::Ptr& commandPool, 
                          lzvk::loader::MeshData& meshData, 
                          lzvk::loader::Scene& scene,
                          int frameCount) {

            return std::make_shared<SceneMeshRenderer>(device, commandPool, meshData, scene, frameCount);
        }

        SceneMeshRenderer(const lzvk::wrapper::Device::Ptr& device, 
                          const lzvk::wrapper::CommandPool::Ptr& commandPool, 
                          lzvk::loader::MeshData& meshData, 
                          lzvk::loader::Scene& scene,
                          int frameCount);

        ~SceneMeshRenderer();

        void draw(const lzvk::wrapper::CommandBuffer::Ptr& cmd);

        std::vector<VkVertexInputBindingDescription> SceneMeshRenderer::getVertexInputBindingDescriptions() {
            return { VkVertexInputBindingDescription{ 0, sizeof(float) * 11, VK_VERTEX_INPUT_RATE_VERTEX } };
        }

        std::vector<VkVertexInputAttributeDescription> SceneMeshRenderer::getAttributeDescriptions() {
            return {

                { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
                { 1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 3 },
                { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 5 },
                { 3, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 8 }
            };
        }

        [[nodiscard]] auto getDescriptorSetLayout_Static() const { return mDescriptorSetLayout_Static; }
        [[nodiscard]] auto getDescriptorSetLayout_Diffuse() const { return mDescriptorSetLayout_Diffuse; }
        [[nodiscard]] auto getDescriptorSetLayout_Emissive() const { return mDescriptorSetLayout_Emissive; }
        [[nodiscard]] auto getDescriptorSetLayout_Normal() const { return mDescriptorSetLayout_Normal; }
        [[nodiscard]] auto getDescriptorSetLayout_Opacity() const { return mDescriptorSetLayout_Opacity; }

        [[nodiscard]] auto getDescriptorSet_Static() const { return mDescriptorSet_Static; }
        [[nodiscard]] auto getDescriptorSet_Diffuse() const { return mDescriptorSet_Diffuse; }
        [[nodiscard]] auto getDescriptorSet_Emissive() const { return mDescriptorSet_Emissive; }
        [[nodiscard]] auto getDescriptorSet_Normal() const { return mDescriptorSet_Normal; }
        [[nodiscard]] auto getDescriptorSet_Opacity() const { return mDescriptorSet_Opacity; }


    private:

        lzvk::wrapper::Device::Ptr mDevice{ nullptr };
        lzvk::wrapper::CommandPool::Ptr mCommandPool{ nullptr };
        lzvk::wrapper::Buffer::Ptr mVertexBuffer{ nullptr };
        lzvk::wrapper::Buffer::Ptr mIndexBuffer{ nullptr };
        lzvk::wrapper::Buffer::Ptr mIndirectBuffer{ nullptr };

        uint32_t mDrawCount{ 0 };

        // Uniform managers
        lzvk::renderer::TransformUniformManager::Ptr mTransformUniformManager{ nullptr };
        lzvk::renderer::MaterialUniformManager::Ptr mMaterialUniformManager{ nullptr };
        lzvk::renderer::DrawDataUniformManager::Ptr mDrawDataUniformManager{ nullptr };
        lzvk::renderer::SceneTextureManager::Ptr mSceneTextureManager{ nullptr };
        
        // descriptors
        lzvk::wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Static{ nullptr };
        lzvk::wrapper::DescriptorPool::Ptr      mDescriptorPool_Static{ nullptr };
        lzvk::wrapper::DescriptorSet::Ptr       mDescriptorSet_Static{ nullptr };

        // diffuse
        lzvk::wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Diffuse{ nullptr };
        lzvk::wrapper::DescriptorPool::Ptr      mDescriptorPool_Diffuse{ nullptr };
        lzvk::wrapper::DescriptorSet::Ptr       mDescriptorSet_Diffuse{ nullptr };

        // emissive
        lzvk::wrapper::DescriptorSet::Ptr mDescriptorSet_Emissive{ nullptr };
        lzvk::wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Emissive{ nullptr };
        lzvk::wrapper::DescriptorPool::Ptr mDescriptorPool_Emissive{ nullptr };

        // normal
        lzvk::wrapper::DescriptorSet::Ptr mDescriptorSet_Normal{ nullptr };
        lzvk::wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Normal{ nullptr };
        lzvk::wrapper::DescriptorPool::Ptr mDescriptorPool_Normal{ nullptr };

        // opacity
        lzvk::wrapper::DescriptorSet::Ptr mDescriptorSet_Opacity{ nullptr };
        lzvk::wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Opacity{ nullptr };
        lzvk::wrapper::DescriptorPool::Ptr mDescriptorPool_Opacity{ nullptr };

    };

}

