#pragma once

#include "../loader/scene.h"
#include "../loader/mesh.h"

namespace lzvk::tools {

    void mergeNodesWithMaterial(lzvk::loader::Scene& scene, lzvk::loader::MeshData& meshData, const std::string& materialName);
    
    void mergeScenes(lzvk::loader::Scene& mergedScene,
                    const std::vector<lzvk::loader::Scene*>& scenes,
                    const std::vector<glm::mat4>& rootTransforms,
                    const std::vector<uint32_t>& meshCounts);

    void shiftNodes(lzvk::loader::Scene& scene, int startOffset, int nodeCount, int shiftAmount);
    void mergeMeshData(
        lzvk::loader::MeshData& out,
        const lzvk::loader::MeshData& a,
        const lzvk::loader::MeshData& b);

    void mergeMaterialLists(
        lzvk::loader::MeshData& merged,
        const std::vector<lzvk::loader::MeshData*>& meshList);
}