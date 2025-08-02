#include "scene_tools.h"

namespace lzvk::tools {

    void mergeNodesWithMaterial(
        lzvk::loader::Scene& scene,
        lzvk::loader::MeshData& meshData,
        const std::string& materialName)
    {
        using namespace lzvk::loader;

        std::cout << "=== mergeNodesWithMaterial: " << materialName << " ===" << std::endl;

        // 1 find material name
        auto it = std::find(scene.materialNames.begin(), scene.materialNames.end(), materialName);
        if (it == scene.materialNames.end())
        {
            std::cout << "[merge] Material not found: " << materialName << std::endl;
            return;
        }

        // 2 find material id
        uint32_t materialId = static_cast<uint32_t>(std::distance(scene.materialNames.begin(), it));
        std::cout << "[merge] Material ID: " << materialId << std::endl;

        // 3 collect all draw data and meshes that contains this material
        std::vector<uint32_t> nodesToMerge;
        std::vector<uint32_t> meshesToMerge;

        for (size_t i = 0; i < scene.drawDataArray.size(); ++i)
        {
            const auto& dd = scene.drawDataArray[i];
            if (dd.materialId == materialId)
            {
                nodesToMerge.push_back(dd.transformId);

                auto itMesh = scene.meshForNode.find(dd.transformId);
                if (itMesh != scene.meshForNode.end())
                {
                    meshesToMerge.push_back(itMesh->second);
                }
            }
        }

        if (meshesToMerge.size() <= 1)
        {
            std::cout << "[merge] Nothing to merge for material: " << materialName << std::endl;
            return;
        }

        std::cout << "[merge] Found " << meshesToMerge.size() << " meshes to merge." << std::endl;

        // 4 create new mesh
        Mesh mergedMesh;
        mergedMesh.materialID = materialId;
        mergedMesh.vertexOffset = static_cast<uint32_t>(meshData.vertexData.size() / (sizeof(float) * 11));
        mergedMesh.indexOffset = static_cast<uint32_t>(meshData.indexData.size());
        mergedMesh.indexCount = 0;

        uint32_t vertexOffsetShift = 0;

        for (auto meshIdx : meshesToMerge)
        {
            const auto& mesh = meshData.meshes[meshIdx];

            uint32_t startVertex = mesh.vertexOffset * 11;
            uint32_t endVertex = startVertex + mesh.indexCount * 11;

            // copy vertices
            meshData.vertexData.insert(
                meshData.vertexData.end(),
                meshData.vertexData.begin() + startVertex * sizeof(float),
                meshData.vertexData.begin() + endVertex * sizeof(float)
            );

            // copy indices and do offset shift
            for (size_t i = 0; i < mesh.indexCount; ++i)
            {
                uint32_t index = meshData.indexData[mesh.indexOffset + i];
                index += vertexOffsetShift;
                meshData.indexData.push_back(index);
            }

            vertexOffsetShift += mesh.indexCount;
            mergedMesh.indexCount += mesh.indexCount;
        }

        meshData.meshes.push_back(mergedMesh);
        uint32_t mergedMeshIdx = static_cast<uint32_t>(meshData.meshes.size() - 1);

        // 5 add new node
        int newNodeId = addNode(scene, -1, 1);
        scene.nodeNames.push_back("Merged_" + materialName);
        scene.nameForNode[newNodeId] = static_cast<uint32_t>(scene.nodeNames.size() - 1);
        scene.meshForNode[newNodeId] = mergedMeshIdx;

        // 6 add new draw data
        DrawData newDD;
        newDD.transformId = newNodeId;
        newDD.materialId = materialId;
        scene.drawDataArray.push_back(newDD);

        std::cout << "[merge] Merged mesh and node created for material: " << materialName << std::endl;

        // 7 remove old drawData
        scene.drawDataArray.erase(
            std::remove_if(
                scene.drawDataArray.begin(),
                scene.drawDataArray.end(),
                [materialId](const DrawData& dd)
                {
                    return dd.materialId == materialId;
                }),
            scene.drawDataArray.end()
        );

        std::cout << "[merge] Old nodes removed for material: " << materialName << std::endl;
    }

    void mergeScenes(
        lzvk::loader::Scene& mergedScene,
        const std::vector<lzvk::loader::Scene*>& scenes,
        const std::vector<glm::mat4>& rootTransforms,
        const std::vector<uint32_t>& meshCounts)
    {
        using namespace lzvk::loader;

        if (scenes.empty()) return;

        // root
        lzvk::loader::Hierarchy h;
        h.parent = -1;
        h.firstChild = scenes.size() > 0 ? 1 : -1;
        h.nextSibling = -1;
        h.lastSibling = -1;
        h.level = 0;
        mergedScene.hierarchy.push_back(h);
        mergedScene.localTransform.push_back(glm::mat4(1.0f));
        mergedScene.globalTransform.push_back(glm::mat4(1.0f));

        mergedScene.nodeNames.push_back("NewRoot");
        mergedScene.nameForNode[0] = 0;

        int nodeOffset = 1;
        int meshOffset = 0;
        int materialOffset = 0;
        int nameOffset = (int)mergedScene.nodeNames.size();
        auto meshCountIt = meshCounts.begin();

        for (size_t sIdx = 0; sIdx < scenes.size(); ++sIdx)
        {
            const Scene* s = scenes[sIdx];

            // append transforms
            mergedScene.localTransform.insert(
                mergedScene.localTransform.end(),
                s->localTransform.begin(),
                s->localTransform.end()
            );
            mergedScene.globalTransform.insert(
                mergedScene.globalTransform.end(),
                s->globalTransform.begin(),
                s->globalTransform.end()
            );

            // append hierarchy
            mergedScene.hierarchy.insert(
                mergedScene.hierarchy.end(),
                s->hierarchy.begin(),
                s->hierarchy.end()
            );

            // append node names
            mergedScene.nodeNames.insert(
                mergedScene.nodeNames.end(),
                s->nodeNames.begin(),
                s->nodeNames.end()
            );

            // append material names
            mergedScene.materialNames.insert(
                mergedScene.materialNames.end(),
                s->materialNames.begin(),
                s->materialNames.end()
            );

            int nodeCount = (int)s->hierarchy.size();

            // shift hierarchy indices
            shiftNodes(mergedScene, nodeOffset, nodeCount, nodeOffset);
            for (int i = nodeOffset; i < nodeOffset + nodeCount; ++i)
                mergedScene.hierarchy[i].level += 1;
            
            // merge maps
            auto mergeMap = [&](auto& mergedMap, const auto& map, int indexOffset, int itemOffset) {
                for (const auto& [k, v] : map)
                {
                    mergedMap[k + indexOffset] = v + itemOffset;
                }
                };

            mergeMap(mergedScene.meshForNode, s->meshForNode, nodeOffset, meshOffset);
            mergeMap(mergedScene.materialForNode, s->materialForNode, nodeOffset, materialOffset);
            mergeMap(mergedScene.nameForNode, s->nameForNode, nodeOffset, nameOffset);

            // merge drawDataArray
            for (const auto& dd : s->drawDataArray)
            {
                DrawData newDD;
                newDD.transformId = dd.transformId + nodeOffset;
                newDD.materialId = dd.materialId + materialOffset;
                mergedScene.drawDataArray.push_back(newDD);
            }

            // fix sibling + parent of old scene root
            int oldRoot = nodeOffset;
            if (rootTransforms.size() > sIdx)
            {
                mergedScene.localTransform[oldRoot] = rootTransforms[sIdx] * mergedScene.localTransform[oldRoot];
            }

            mergedScene.hierarchy[oldRoot].parent = 0;
            mergedScene.hierarchy[oldRoot].nextSibling = -1;

            nodeOffset += nodeCount;
            materialOffset += (int)s->materialNames.size();
            nameOffset += (int)s->nodeNames.size();
            if (meshCountIt != meshCounts.end())
            {
                meshOffset += *meshCountIt;
                ++meshCountIt;
            }
        }

        //  sibling
        for (size_t sIdx = 0; sIdx < scenes.size(); ++sIdx)
        {
            int childIndex = 1;
            for (size_t i = 0; i < sIdx; ++i)
                childIndex += scenes[i]->hierarchy.size();

            if (sIdx < scenes.size() - 1)
            {
                mergedScene.hierarchy[childIndex].nextSibling =
                    childIndex + (int)scenes[sIdx]->hierarchy.size();
            }
            else
            {
                mergedScene.hierarchy[childIndex].nextSibling = -1;
            }
        }

        std::cout << "[mergeScenes] Completed merging " << scenes.size() << " scenes into one.\n";
    }

    void shiftNodes(lzvk::loader::Scene& scene, int startOffset, int nodeCount, int shiftAmount)
    {
        for (int i = 0; i < nodeCount; i++) {
            auto& h = scene.hierarchy[i + startOffset];
            if (h.parent > -1) h.parent += shiftAmount;
            if (h.firstChild > -1) h.firstChild += shiftAmount;
            if (h.nextSibling > -1) h.nextSibling += shiftAmount;
            if (h.lastSibling > -1) h.lastSibling += shiftAmount;
        }
    }

    // --- mergeMeshData:  ---
    void mergeMeshData(
        lzvk::loader::MeshData& out,
        const lzvk::loader::MeshData& a,
        const lzvk::loader::MeshData& b)
    {
        using namespace lzvk::loader;

        out = MeshData();

        const uint32_t vertexStride = 44;  // float[11]
        const uint32_t indexOffset = static_cast<uint32_t>(a.indexData.size());
        const uint32_t vertexOffset = static_cast<uint32_t>(a.vertexData.size()) / (sizeof(float) * 11);

        // ---- Step 1:  a index/vertex/mesh ----
        out.indexData = a.indexData;
        out.vertexData = a.vertexData;
        out.meshes = a.meshes;

        // ---- Step 2:  b  index ----
        out.indexData.insert(out.indexData.end(), b.indexData.begin(), b.indexData.end());

        // ---- Step 3:  b  vertex ----
        out.vertexData.insert(out.vertexData.end(), b.vertexData.begin(), b.vertexData.end());

        // ---- Step 4:  b  mesh (index/vertex offset) ----
        for (const Mesh& mesh : b.meshes)
        {
            Mesh m = mesh;
            m.indexOffset += indexOffset;
            m.vertexOffset = mesh.vertexOffset + vertexOffset;
            out.meshes.push_back(m);
        }

        printf("[mergeMeshData] merged meshes = %zu, indices = %zu, vertexData = %zu bytes\n",
            out.meshes.size(), out.indexData.size(), out.vertexData.size());
    }


    // --- mergeMaterialLists  ---
    void mergeMaterialLists(
        lzvk::loader::MeshData& merged,
        const std::vector<lzvk::loader::MeshData*>& meshList)
    {
        using namespace lzvk::loader;
        using namespace std;

        // 
        merged.materials.clear();
        merged.diffuseTextureFiles.clear();
        merged.normalTextureFiles.clear();
        merged.emissiveTextureFiles.clear();
        merged.opacityTextureFiles.clear();

        //
        vector<Material> allMaterials;
        unordered_map<size_t, size_t> materialToSourceMap;

        vector<const vector<string>*> diffuseLists, normalLists, emissiveLists, opacityLists;

        for (size_t i = 0; i < meshList.size(); ++i) {
            const auto* meshData = meshList[i];

            diffuseLists.push_back(&meshData->diffuseTextureFiles);
            normalLists.push_back(&meshData->normalTextureFiles);
            emissiveLists.push_back(&meshData->emissiveTextureFiles);
            opacityLists.push_back(&meshData->opacityTextureFiles);

            for (const auto& mat : meshData->materials) {
                allMaterials.push_back(mat);
                materialToSourceMap[allMaterials.size() - 1] = i;
            }
        }

        // 3.
        auto buildUnifiedTextureList = [](const vector<const vector<string>*>& sources, vector<string>& unifiedList) {
            unordered_map<string, int> map;
            for (const auto* list : sources) {
                for (const auto& path : *list) {
                    if (map.find(path) == map.end()) {
                        map[path] = static_cast<int>(unifiedList.size());
                        unifiedList.push_back(path);
                    }
                }
            }
            return map;
            };

        auto diffuseMap = buildUnifiedTextureList(diffuseLists, merged.diffuseTextureFiles);
        auto normalMap = buildUnifiedTextureList(normalLists, merged.normalTextureFiles);
        auto emissiveMap = buildUnifiedTextureList(emissiveLists, merged.emissiveTextureFiles);
        auto opacityMap = buildUnifiedTextureList(opacityLists, merged.opacityTextureFiles);

        // 4
        auto remapIndex = [&](size_t matIndex, uint32_t& index, const vector<const vector<string>*>& texLists, const unordered_map<string, int>& map) {
            if (index == uint32_t(-1)) return;

            size_t srcMesh = materialToSourceMap[matIndex];
            const auto& list = *texLists[srcMesh];

            if (index >= list.size()) {
                index = uint32_t(-1);
                return;
            }

            const string& texPath = list[index];
            auto it = map.find(texPath);
            if (it != map.end())
                index = static_cast<uint32_t>(it->second);
            else
                index = uint32_t(-1);
            };

        for (size_t i = 0; i < allMaterials.size(); ++i) {
            Material& m = allMaterials[i];

            remapIndex(i, m.baseColorTexture, diffuseLists, diffuseMap);
            remapIndex(i, m.normalTexture, normalLists, normalMap);
            remapIndex(i, m.emissiveTexture, emissiveLists, emissiveMap);
            remapIndex(i, m.opacityTexture, opacityLists, opacityMap);

        }

        merged.materials = allMaterials;

        size_t meshOffset = 0;
        size_t materialOffset = 0;
        for (size_t i = 0; i < meshList.size(); ++i) {
            const auto* src = meshList[i];
            const size_t localMatCount = src->materials.size();
            for (size_t j = 0; j < src->meshes.size(); ++j) {
                merged.meshes[meshOffset + j].materialID += static_cast<uint32_t>(materialOffset);
            }
            meshOffset += src->meshes.size();
            materialOffset += localMatCount;
        }

        printf("[mergeMaterialLists] After merge: materials = %zu, diffuse = %zu, normal = %zu, emissive = %zu, opacity = %zu\n",
            merged.materials.size(),
            merged.diffuseTextureFiles.size(),
            merged.normalTextureFiles.size(),
            merged.emissiveTextureFiles.size(),
            merged.opacityTextureFiles.size());
    }





}
