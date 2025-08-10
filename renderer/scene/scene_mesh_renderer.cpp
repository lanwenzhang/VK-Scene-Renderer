#include "scene_mesh_renderer.h"
#include <cstring>
#include <stdexcept>
#include <filesystem>


namespace lzvk::renderer {

    SceneMeshRenderer::SceneMeshRenderer(const lzvk::wrapper::Device::Ptr& device, 
                                         const lzvk::wrapper::CommandPool::Ptr& commandPool, 
                                         lzvk::loader::MeshData& meshData,
                                         lzvk::loader::Scene& scene,
                                         int frameCount) {

        mDevice = device;
        mCommandPool = commandPool;

        // Create vertex buffer
        mVertexBuffer = lzvk::wrapper::Buffer::createVertexBuffer(
            mDevice,
            meshData.vertexData.size(),
            meshData.vertexData.data()
        );

        // Create index buffer
        mIndexBuffer = lzvk::wrapper::Buffer::createIndexBuffer(
            mDevice,
            meshData.indexData.size() * sizeof(uint32_t),
            meshData.indexData.data()
        );


        //
       // ========== STATIC SET ==========
       //

       // Create TransformUniformManager
        mTransformUniformManager = lzvk::renderer::TransformUniformManager::create();
        mTransformUniformManager->init(
            mDevice,
            scene.globalTransform.size(),
            scene.globalTransform.data(),
            frameCount
        );


        // Create MaterialUniformManager
        mMaterialUniformManager = lzvk::renderer::MaterialUniformManager::create();
        mMaterialUniformManager->init(
            mDevice,
            meshData.materials.size(),
            meshData.materials.data(),
            frameCount
        );

        // Create DrawDataUniformManager
        mDrawDataUniformManager = lzvk::renderer::DrawDataUniformManager::create();
        mDrawDataUniformManager->init(
            mDevice,
            scene.drawDataArray.size(),
            scene.drawDataArray.data(),
            frameCount
        );

        // Create Static DescriptorSet (set = 1)
        std::vector<lzvk::wrapper::UniformParameter::Ptr> staticParams;

        auto append = [&](const std::vector<lzvk::wrapper::UniformParameter::Ptr>& params) {
            staticParams.insert(staticParams.end(), params.begin(), params.end());
            };

        append(mTransformUniformManager->getParams());
        append(mMaterialUniformManager->getParams());
        append(mDrawDataUniformManager->getParams());

        mDescriptorSetLayout_Static = lzvk::wrapper::DescriptorSetLayout::create(mDevice);
        mDescriptorSetLayout_Static->build(staticParams);

        mDescriptorPool_Static = lzvk::wrapper::DescriptorPool::create(mDevice);
        mDescriptorPool_Static->build(staticParams, 1);

        mDescriptorSet_Static = lzvk::wrapper::DescriptorSet::create(
            mDevice,
            staticParams,
            mDescriptorSetLayout_Static,
            mDescriptorPool_Static,
            1
        );

        //
        // ========== DIFFUSE TEXTURE SET ==========
        //

        mSceneTextureManager = lzvk::renderer::SceneTextureManager::create();
        mSceneTextureManager->init(
            mDevice,
            mCommandPool,
            meshData,
            frameCount
        );

        //
        // Diffuse descriptor set (set = 2)
        //
        auto diffuseParams = mSceneTextureManager->getDiffuseParams();

        mDescriptorSetLayout_Diffuse = lzvk::wrapper::DescriptorSetLayout::create(mDevice);
        mDescriptorSetLayout_Diffuse->build(diffuseParams);

        mDescriptorPool_Diffuse = lzvk::wrapper::DescriptorPool::create(mDevice);
        mDescriptorPool_Diffuse->build(diffuseParams, 1);

        mDescriptorSet_Diffuse = lzvk::wrapper::DescriptorSet::create(
            mDevice,
            diffuseParams,
            mDescriptorSetLayout_Diffuse,
            mDescriptorPool_Diffuse,
            1
        );

        //
        // ========== EMISSIVE TEXTURE SET ==========
        //

        auto emissiveParams = mSceneTextureManager->getEmissiveParams();

        mDescriptorSetLayout_Emissive = lzvk::wrapper::DescriptorSetLayout::create(mDevice);
        mDescriptorSetLayout_Emissive->build(emissiveParams);

        mDescriptorPool_Emissive = lzvk::wrapper::DescriptorPool::create(mDevice);
        mDescriptorPool_Emissive->build(emissiveParams, 1);

        mDescriptorSet_Emissive = lzvk::wrapper::DescriptorSet::create(
            mDevice,
            emissiveParams,
            mDescriptorSetLayout_Emissive,
            mDescriptorPool_Emissive,
            1
        );

        //
        // ========== NORMAL TEXTURE SET ==========
        //

        auto normalParams = mSceneTextureManager->getNormalParams();

        mDescriptorSetLayout_Normal = lzvk::wrapper::DescriptorSetLayout::create(mDevice);
        mDescriptorSetLayout_Normal->build(normalParams);

        mDescriptorPool_Normal = lzvk::wrapper::DescriptorPool::create(mDevice);
        mDescriptorPool_Normal->build(normalParams, 1);

        mDescriptorSet_Normal = lzvk::wrapper::DescriptorSet::create(
            mDevice,
            normalParams,
            mDescriptorSetLayout_Normal,
            mDescriptorPool_Normal,
            1
        );

        //
        // ========== OPACITY TEXTURE SET ==========
        //

        auto opacityParams = mSceneTextureManager->getOpacityParams();

        mDescriptorSetLayout_Opacity = lzvk::wrapper::DescriptorSetLayout::create(mDevice);
        mDescriptorSetLayout_Opacity->build(opacityParams);

        mDescriptorPool_Opacity = lzvk::wrapper::DescriptorPool::create(mDevice);
        mDescriptorPool_Opacity->build(opacityParams, 1);

        mDescriptorSet_Opacity = lzvk::wrapper::DescriptorSet::create(
            mDevice,
            opacityParams,
            mDescriptorSetLayout_Opacity,
            mDescriptorPool_Opacity,
            1
        );

        //
        // ========== SPECULAR TEXTURE SET ==========
        //

        auto specularParams = mSceneTextureManager->getSpecularParams();

        mDescriptorSetLayout_Specular = lzvk::wrapper::DescriptorSetLayout::create(mDevice);
        mDescriptorSetLayout_Specular->build(specularParams);

        mDescriptorPool_Specular = lzvk::wrapper::DescriptorPool::create(mDevice);
        mDescriptorPool_Specular->build(specularParams, 1);

        mDescriptorSet_Specular = lzvk::wrapper::DescriptorSet::create(
            mDevice,
            specularParams,
            mDescriptorSetLayout_Specular,
            mDescriptorPool_Specular,
            1
        );



        //
        // ========== INDIRECT BUFFER ==========
        //

        std::vector<VkDrawIndexedIndirectCommand> drawCommands;

        for (size_t i = 0; i < scene.drawDataArray.size(); ++i) {

            const lzvk::loader::DrawData& dd = scene.drawDataArray[i];

            auto it = scene.meshForNode.find(dd.transformId);

            if (it != scene.meshForNode.end()) {

                uint32_t meshIdx = it->second;
                const  lzvk::loader::Mesh& mesh = meshData.meshes[meshIdx];

                VkDrawIndexedIndirectCommand cmd{};
                cmd.indexCount = mesh.indexCount;
                cmd.instanceCount = 1;
                cmd.firstIndex = mesh.indexOffset;
                cmd.vertexOffset = mesh.vertexOffset;
                cmd.firstInstance = static_cast<uint32_t>(i);

                drawCommands.push_back(cmd);
            }
        }

        mDrawCount = static_cast<uint32_t>(drawCommands.size());


        mIndirectBuffer = lzvk::wrapper::Buffer::createStorageBuffer(
            device,
            drawCommands.size() * sizeof(VkDrawIndexedIndirectCommand),
            drawCommands.data(),
            false
        );
    }


    SceneMeshRenderer::~SceneMeshRenderer() {}

    void SceneMeshRenderer::draw(const lzvk::wrapper::CommandBuffer::Ptr& cmd) {

        cmd->bindVertexBuffer({ mVertexBuffer->getBuffer() });
        cmd->bindIndexBuffer(mIndexBuffer->getBuffer());
        cmd->drawIndexedIndirect(mIndirectBuffer->getBuffer(), 0, mDrawCount, sizeof(VkDrawIndexedIndirectCommand));
    }

}


