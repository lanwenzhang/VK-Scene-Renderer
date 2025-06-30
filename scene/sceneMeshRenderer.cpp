#include "sceneMeshRenderer.h"
#include <cstring>
#include <stdexcept>

SceneMeshRenderer::SceneMeshRenderer(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, MeshData& meshData, const Scene& scene){

	mDevice = device;
    mCommandPool = commandPool;

    // Create vertex buffer
    mVertexBuffer = Wrapper::Buffer::createVertexBuffer(
        mDevice,
        meshData.vertexData.size(),
        meshData.vertexData.data()
    );

    // Create index buffer
    mIndexBuffer = Wrapper::Buffer::createIndexBuffer(
        mDevice,
        meshData.indexData.size() * sizeof(uint32_t),
        meshData.indexData.data()
    );

    loadTextures(mDevice, mCommandPool, meshData);


    // Create draw indirect buffer
    std::unordered_map<uint32_t, uint32_t> nodeIdToDrawDataIndex;
    for (size_t i = 0; i < scene.drawDataArray.size(); ++i) {
        nodeIdToDrawDataIndex[scene.drawDataArray[i].transformId] = static_cast<uint32_t>(i);
    }


    std::vector<VkDrawIndexedIndirectCommand> drawCommands;
    for (size_t i = 0; i < scene.drawDataArray.size(); ++i) {
        const DrawData& dd = scene.drawDataArray[i];
        const Mesh& mesh = meshData.meshes[i];

        VkDrawIndexedIndirectCommand cmd{};
        cmd.indexCount = mesh.indexCount;
        cmd.instanceCount = 1;
        cmd.firstIndex = mesh.indexOffset;
        cmd.vertexOffset = mesh.vertexOffset;
        cmd.firstInstance = static_cast<uint32_t>(i);

        std::cout << "DrawCmd[" << drawCommands.size() << "] "
            << "firstInstance = " << cmd.firstInstance
            << ", indexCount = " << cmd.indexCount
            << ", firstIndex = " << cmd.firstIndex
            << ", vertexOffset = " << cmd.vertexOffset
            << std::endl;

        drawCommands.push_back(cmd);
    }

    mDrawCount = static_cast<uint32_t>(drawCommands.size());

    mIndirectBuffer = Wrapper::Buffer::createStorageBuffer(
        device,
        drawCommands.size() * sizeof(VkDrawIndexedIndirectCommand),
        drawCommands.data(),
        false
    );
}

void SceneMeshRenderer::loadTextures(const Wrapper::Device::Ptr& device,
    const Wrapper::CommandPool::Ptr& commandPool,
    MeshData& meshData)
{
    mDiffuseTextures.clear();

    auto loadTextureArray = [&](const std::vector<std::string>& texFiles,
        std::vector<Texture::Ptr>& textures,
        const std::string& prefix) {
            for (const auto& path : texFiles) {
                std::string fullPath = prefix + path;
                try {
                    auto texture = Texture::create(device, commandPool, fullPath);
                    textures.push_back(texture);
                }
                catch (const std::exception& e) {
                    std::cerr << "Failed to load texture: "
                        << fullPath << " - " << e.what() << std::endl;
                    textures.push_back(nullptr);
                }
            }
        };

    loadTextureArray(meshData.diffuseTextureFiles, mDiffuseTextures, "assets/crytek_sponza/");

    std::cout << "Loaded diffuse textures: " << mDiffuseTextures.size() << std::endl;
}

SceneMeshRenderer::~SceneMeshRenderer(){}

void SceneMeshRenderer::draw(const Wrapper::CommandBuffer::Ptr& cmd) {

    cmd->bindVertexBuffer({ mVertexBuffer->getBuffer() });
    cmd->bindIndexBuffer(mIndexBuffer->getBuffer());
    cmd->drawIndexedIndirect(mIndirectBuffer->getBuffer(), 0, mDrawCount, sizeof(VkDrawIndexedIndirectCommand));
}
