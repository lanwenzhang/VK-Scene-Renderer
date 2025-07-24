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
        const uint32_t vertexOffset = static_cast<uint32_t>(a.vertexData.size()) / vertexStride;

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

        //for (size_t i = a.meshes.size(); i < out.meshes.size(); ++i) {
        //    const auto& mesh = out.meshes[i];
        //    uint32_t vStart = mesh.vertexOffset;
        //    uint32_t iStart = mesh.indexOffset;
        //    uint32_t iCount = mesh.indexCount;
        //    size_t vertexCount = out.vertexData.size() / vertexStride;

        //    for (uint32_t j = 0; j < iCount; ++j) {
        //        uint32_t idx = out.indexData[iStart + j];
        //        size_t byteOffset = size_t(idx + vStart) * vertexStride;
        //        if (byteOffset + vertexStride > out.vertexData.size()) {
        //            printf(" Mesh %zu: index %u (vertex = %u) out of bounds! Byte offset = %zu, vertexData.size = %zu\n",
        //                i, j, idx + vStart, byteOffset, out.vertexData.size());
        //            break;
        //        }
        //    }
        //}
    }


    // --- mergeMaterialLists  ---
    void mergeMaterialLists(
        lzvk::loader::MeshData& merged,
        const std::vector<lzvk::loader::MeshData*>& meshList)
    {
        using namespace lzvk::loader;
        using namespace std;

        printf("[mergeMaterialLists] Begin material merge...\n");

        size_t materialOffset = 0;
        std::vector<uint32_t> materialOffsets;
        for (const auto* p : meshList) {
            materialOffsets.push_back(static_cast<uint32_t>(materialOffset));
            materialOffset += static_cast<uint32_t>(p->materials.size());
        }

        // Count before merge
        size_t totalMaterials = 0;
        size_t totalDiffuse = 0, totalNormal = 0, totalEmissive = 0, totalOcclusion = 0;
        for (const auto* p : meshList) {
            totalMaterials += p->materials.size();
            totalDiffuse += p->diffuseTextureFiles.size();
            totalNormal += p->normalTextureFiles.size();
            totalEmissive += p->emissiveTextureFiles.size();
            totalOcclusion += p->occlusionTextureFiles.size();
        }

        printf("[mergeMaterialLists] Before merge: materials = %zu, diffuse = %zu, normal = %zu, emissive = %zu, occlusion = %zu\n",
            totalMaterials, totalDiffuse, totalNormal, totalEmissive, totalOcclusion);

        // Texture path deduplication
        unordered_map<string, int> diffuseMap, normalMap, emissiveMap, occlusionMap;

        auto sanitizePath = [](std::string path) -> std::string {
            std::replace(path.begin(), path.end(), '\\', '/'); // backslash to slash
            while (path.rfind("../", 0) == 0 || path.rfind("./", 0) == 0)
                path = path.substr(path.find_first_of('/') + 1);
            return path;
            };

        auto validatePath = [&sanitizePath](const std::string& label, const std::string& rawPath) {
            if (rawPath.empty() || rawPath == "dummy.png") return;
            std::string path = sanitizePath(rawPath);

            std::ifstream f(path.c_str());
            if (!f.good()) {
                printf("[Warning] %s texture missing: %s\n", label.c_str(), path.c_str());
            }
            };


        auto addUnique = [&sanitizePath](vector<string>& dst, unordered_map<string, int>& map, const string& rawPath) -> int {
            if (rawPath.empty() || rawPath == "dummy.png") return -1;

            std::string path = sanitizePath(rawPath);
            std::ifstream f(path.c_str());
            if (!f.good()) {
                return -1;
            }

            auto it = map.find(path);
            if (it != map.end()) return it->second;

            int index = static_cast<int>(dst.size());
            dst.push_back(path);
            map[path] = index;
            return index;
            };


        merged.materials.clear();

        for (size_t i = 0; i < meshList.size(); ++i)
        {
            const auto* src = meshList[i];
            for (const auto& mat : src->materials)
            {
                Material m = mat;

                validatePath("Diffuse", mat.baseColorTexturePath);
                validatePath("Normal", mat.normalTexturePath);
                validatePath("Emissive", mat.emissiveTexturePath);
                validatePath("Occlusion", mat.occlusionTexturePath);

                m.baseColorTexture = addUnique(merged.diffuseTextureFiles, diffuseMap, mat.baseColorTexturePath);
                m.normalTexture = addUnique(merged.normalTextureFiles, normalMap, mat.normalTexturePath);
                m.emissiveTexture = addUnique(merged.emissiveTextureFiles, emissiveMap, mat.emissiveTexturePath);
                m.occlusionTexture = addUnique(merged.occlusionTextureFiles, occlusionMap, mat.occlusionTexturePath);
                m.opacityTexture = -1;

                merged.materials.push_back(m);
            }
        }

        // Update mesh materialID
        uint32_t globalMaterialOffset = 0;
        for (size_t i = 0; i < meshList.size(); ++i)
        {
            uint32_t localCount = static_cast<uint32_t>(meshList[i]->materials.size());
            for (uint32_t j = 0; j < localCount; ++j)
            {
                uint32_t localMatID = j;
                uint32_t globalMatID = globalMaterialOffset + j;

                for (auto& mesh : merged.meshes)
                {
                    if (mesh.materialID == localMatID)
                    {
                        mesh.materialID = globalMatID;
                    }
                }
            }
            globalMaterialOffset += localCount;
        }

        printf("[mergeMaterialLists] After merge: materials = %zu, diffuse = %zu, normal = %zu, emissive = %zu, occlusion = %zu\n",
            merged.materials.size(),
            merged.diffuseTextureFiles.size(),
            merged.normalTextureFiles.size(),
            merged.emissiveTextureFiles.size(),
            merged.occlusionTextureFiles.size());
    }

}
