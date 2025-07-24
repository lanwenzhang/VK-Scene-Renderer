#include <assimp/importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include "mesh.h"

namespace lzvk::loader {

    static int addTextureIfUnique(
        std::vector<std::string>& textureFiles,
        std::unordered_map<std::string, int>& textureMap,
        const std::string& texPath) {
        auto it = textureMap.find(texPath);
        if (it != textureMap.end()) {
            return it->second;
        }
        else {
            int id = static_cast<int>(textureFiles.size());
            textureFiles.push_back(texPath);
            textureMap[texPath] = id;
            return id;
        }
    }

    bool loadMeshFile(const std::string& path, MeshData& meshData, Scene& scene) {

        std::string dummyPath = "assets/bistro/dummy.png";

        meshData.diffuseTextureFiles.push_back(dummyPath);
        meshData.emissiveTextureFiles.push_back(dummyPath);
        meshData.normalTextureFiles.push_back(dummyPath);

        std::unordered_map<std::string, int> diffuseTextureMap;
        diffuseTextureMap[dummyPath] = 0;

        std::unordered_map<std::string, int> emissiveTextureMap;
        emissiveTextureMap[dummyPath] = 0;

        std::unordered_map<std::string, int> normalTextureMap;
        normalTextureMap[dummyPath] = 0;

        Assimp::Importer importer;

        const aiScene* aiScene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_JoinIdenticalVertices |
            aiProcess_CalcTangentSpace
        );

        if (!aiScene || aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiScene->mRootNode) {
            std::cerr << "Assimp load failed: " << importer.GetErrorString() << std::endl;
            return false;
        }


        // ========== LOAD MATERIALS ==========

        for (unsigned int i = 0; i < aiScene->mNumMaterials; i++) {
            aiMaterial* aiMat = aiScene->mMaterials[i];

            Material m;
            m.baseColorFactor = glm::vec4(1.0f);
            m.emissiveFactor = glm::vec4(0.0f);
            m.roughness = 1.0f;
            m.metallicFactor = 1.0f;
            m.alphaTest = 0.5f;
            m.transparencyFactor = 1.0f;
            m.baseColorTexture = uint32_t(-1);

            m.emissiveTexture = 0;
            m.normalTexture = 0;
            m.opacityTexture = 0;
            m.specularTexture = 0;

            m.baseColorTexturePath = "";
            m.emissiveTexturePath = "";
            m.normalTexturePath = "";
            m.opacityTexturePath = "";
            m.specularTexturePath = "";


            aiString str;

            // ---------- diffuse ----------
            if (AI_SUCCESS == aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &str)) {
                std::string rawPath = str.C_Str();

                std::filesystem::path texPath(rawPath);
                std::filesystem::path objPath(path);
                std::filesystem::path baseDir = objPath.parent_path();
                std::filesystem::path fullPath = baseDir / texPath;
                fullPath = fullPath.lexically_normal();
                std::string uri = fullPath.string();

                m.baseColorTexture = addTextureIfUnique(
                    meshData.diffuseTextureFiles,
                    diffuseTextureMap,
                    uri
                );
                m.baseColorTexturePath = uri;

                std::cout << "[Material " << i << "] Diffuse texture: " << uri << std::endl;
            }
            else {
                m.baseColorTexture = 0;
                m.baseColorTexturePath = "dummy.png";

                std::cout << "[Material " << i << "] Diffuse texture: dummy.png" << std::endl;
            }

            // ---------- emissive ----------
            if (AI_SUCCESS == aiMat->GetTexture(aiTextureType_EMISSIVE, 0, &str)) {
                std::string rawPath = str.C_Str();
                std::filesystem::path texPath(rawPath);
                std::filesystem::path objPath(path);
                std::filesystem::path baseDir = objPath.parent_path();
                std::filesystem::path fullPath = baseDir / texPath;
                fullPath = fullPath.lexically_normal();
                std::string uri = fullPath.string();

                m.emissiveTexture = addTextureIfUnique(
                    meshData.emissiveTextureFiles,
                    emissiveTextureMap,
                    uri
                );
                m.emissiveTexturePath = uri;

                std::cout << "[Material " << i << "] Emissive texture: " << uri << std::endl;
            }
            else {
                m.emissiveTexture = 0;
                m.emissiveTexturePath = "dummy.png";
                std::cout << "[Material " << i << "] Emissive texture: dummy.png" << std::endl;
            }

            // ---------- normal ----------
            bool normalFound = false;
            if (AI_SUCCESS == aiMat->GetTexture(aiTextureType_NORMALS, 0, &str)) {
                normalFound = true;
            }
            else if (AI_SUCCESS == aiMat->GetTexture(aiTextureType_HEIGHT, 0, &str)) {
                normalFound = true;
            }

            if (normalFound) {
                std::string rawPath = str.C_Str();
                std::filesystem::path texPath(rawPath);
                std::filesystem::path objPath(path);
                std::filesystem::path baseDir = objPath.parent_path();
                std::filesystem::path fullPath = baseDir / texPath;
                fullPath = fullPath.lexically_normal();
                std::string uri = fullPath.string();

                m.normalTexture = addTextureIfUnique(
                    meshData.normalTextureFiles,
                    normalTextureMap,
                    uri
                );
                m.normalTexturePath = uri;

                std::cout << "[Material " << i << "] Normal texture: " << uri << std::endl;
            }
            else {
                m.normalTexture = 0;
                m.normalTexturePath = "dummy.png";
                std::cout << "[Material " << i << "] Normal texture: dummy.png" << std::endl;
            }


   
            aiString name;
            if (aiMat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
                std::string matName = name.C_Str();
                scene.materialNames.push_back(matName);
                std::cout << "[Material " << i << "] name: " << matName << std::endl;
            }
            else {
                std::string fallbackName = "Material_" + std::to_string(i);
                scene.materialNames.push_back(fallbackName);
                std::cout << "[Material " << i << "] no name found, using fallback: " << fallbackName << std::endl;
            }
            meshData.materials.push_back(m);

        }


        // ========== LOAD MESHES + SCENE NODES ==========

        int root = addNode(scene, -1, 0);
        glm::mat4 modelTransform = glm::mat4(1.0f);

        scene.localTransform[root] = modelTransform;
        scene.globalTransform[root] = modelTransform;
        scene.nodeNames.push_back("Root");
        scene.nameForNode[root] = uint32_t(scene.nodeNames.size() - 1);

        size_t vertexStart = 0;

        std::function<void(aiNode*, int, int)> traverse;
        traverse = [&](aiNode* node, int parent, int level) {

            // 1 add node name
            int nodeId = addNode(scene, parent, level);
            scene.nodeNames.push_back(node->mName.C_Str());
            scene.nameForNode[nodeId] = uint32_t(scene.nodeNames.size() - 1);

            // 2 add node transformation
            aiMatrix4x4 m = node->mTransformation;
            glm::mat4 local(1.0f);
            local = glm::transpose(glm::make_mat4(&m.a1));
            scene.localTransform[nodeId] = local;
            scene.globalTransform[nodeId] = glm::mat4(1.0f);

            // 3 add node meshes
            for (unsigned int meshIndex = 0; meshIndex < node->mNumMeshes; meshIndex++) {
                
                // 3.1 add subnode for this mesh
                aiMesh* aiMesh = aiScene->mMeshes[node->mMeshes[meshIndex]];

                // 3.1.1 subnode name
                int meshNodeId = addNode(scene, nodeId, level + 1);
                std::string meshNodeName = node->mName.C_Str() + std::string("_Mesh_") + std::to_string(meshIndex);
                scene.nodeNames.push_back(meshNodeName);
                scene.nameForNode[meshNodeId] = uint32_t(scene.nodeNames.size() - 1);

                // 3.1.2 subnode transform
                scene.localTransform[meshNodeId] = glm::mat4(1.0f);
                scene.globalTransform[meshNodeId] = glm::mat4(1.0f);

                // 3.1.3 subnode mesh
                Mesh m;
                m.vertexOffset = uint32_t(vertexStart);
                m.materialID = aiMesh->mMaterialIndex;

                for (unsigned int v = 0; v < aiMesh->mNumVertices; v++) {
                    glm::vec3 pos(0.0f);
                    if (aiMesh->HasPositions()) {
                        pos.x = aiMesh->mVertices[v].x;
                        pos.y = aiMesh->mVertices[v].y;
                        pos.z = aiMesh->mVertices[v].z;
                    }

                    glm::vec2 uv(0.0f);
                    if (aiMesh->HasTextureCoords(0)) {
                        uv.x = aiMesh->mTextureCoords[0][v].x;
                        uv.y = aiMesh->mTextureCoords[0][v].y;
                    }

                    glm::vec3 normal(0.0f, 1.0f, 0.0f);
                    if (aiMesh->HasNormals()) {
                        normal.x = aiMesh->mNormals[v].x;
                        normal.y = aiMesh->mNormals[v].y;
                        normal.z = aiMesh->mNormals[v].z;
                    }

                    glm::vec3 tangent(1.0f, 0.0f, 0.0f);
                    if (aiMesh->HasTangentsAndBitangents()) {
                        tangent.x = aiMesh->mTangents[v].x;
                        tangent.y = aiMesh->mTangents[v].y;
                        tangent.z = aiMesh->mTangents[v].z;
                    }

                    float vertex[11] = {
                         pos.x, pos.y, pos.z,
                         uv.x, uv.y,
                         normal.x, normal.y, normal.z,
                         tangent.x, tangent.y, tangent.z
                    };

                    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(vertex);
                    meshData.vertexData.insert(
                        meshData.vertexData.end(),
                        ptr,
                        ptr + sizeof(vertex)
                    );
                    ++vertexStart;
                }

                m.indexOffset = uint32_t(meshData.indexData.size());
                m.indexCount = 0;

                for (unsigned int f = 0; f < aiMesh->mNumFaces; f++) {
                    const aiFace& face = aiMesh->mFaces[f];
                    for (unsigned int j = 0; j < face.mNumIndices; j++) {
                        meshData.indexData.push_back(face.mIndices[j]);
                        m.indexCount++;
                    }
                }

                meshData.meshes.push_back(m);

                // add drawdata for each mesh
                DrawData dd;
                dd.transformId = meshNodeId;
                dd.materialId = m.materialID;
                scene.drawDataArray.push_back(dd);

                scene.meshForNode[meshNodeId] = uint32_t(meshData.meshes.size() - 1);
                scene.materialForNode[meshNodeId] = m.materialID;
            }

            // traverse 
            for (unsigned int i = 0; i < node->mNumChildren; i++) {
                
                traverse(node->mChildren[i], nodeId, level + 1);
            }
        };

        traverse(aiScene->mRootNode, root, 1);

        std::cout << "After loading draw data size "  << scene.drawDataArray.size()<< std::endl;

        recalculateGlobalTransforms(scene);

        return true;
    }

    void saveMeshData(const std::string& path, const MeshData& meshData) {
        FILE* f = fopen(path.c_str(), "wb");
        if (!f) {
            printf("Failed to open file %s for writing\n", path.c_str());
            return;
        }

        auto saveString = [](FILE* f, const std::string& s) {
            uint64_t len = s.length();
            fwrite(&len, sizeof(len), 1, f);
            fwrite(s.c_str(), 1, len, f);
            };

        auto saveMaterialList = [&](const std::vector<Material>& materials) {
            uint64_t count = materials.size();
            fwrite(&count, sizeof(count), 1, f);
            for (const auto& m : materials) {
                fwrite(&m.emissiveFactor, sizeof(glm::vec4), 1, f);
                fwrite(&m.baseColorFactor, sizeof(glm::vec4), 1, f);
                fwrite(&m.roughness, sizeof(float), 1, f);
                fwrite(&m.metallicFactor, sizeof(float), 1, f);
                fwrite(&m.alphaTest, sizeof(float), 1, f);
                fwrite(&m.transparencyFactor, sizeof(float), 1, f);
                fwrite(&m.baseColorTexture, sizeof(uint32_t), 1, f);
                fwrite(&m.specularTexture, sizeof(uint32_t), 1, f);
                fwrite(&m.emissiveTexture, sizeof(uint32_t), 1, f);
                fwrite(&m.normalTexture, sizeof(uint32_t), 1, f);
                fwrite(&m.opacityTexture, sizeof(uint32_t), 1, f);
                fwrite(&m.occlusionTexture, sizeof(uint32_t), 1, f);
                saveString(f, m.baseColorTexturePath);
                saveString(f, m.specularTexturePath);
                saveString(f, m.emissiveTexturePath);
                saveString(f, m.normalTexturePath);
                saveString(f, m.opacityTexturePath);
                saveString(f, m.occlusionTexturePath);
            }
            };

        auto saveStringList = [&](const std::vector<std::string>& list) {
            uint64_t count = list.size();
            fwrite(&count, sizeof(count), 1, f);
            for (const auto& s : list) saveString(f, s);
            };

        uint64_t numMeshes = meshData.meshes.size();
        fwrite(&numMeshes, sizeof(numMeshes), 1, f);
        if (numMeshes > 0)
            fwrite(meshData.meshes.data(), sizeof(Mesh), numMeshes, f);

        uint64_t vertexDataSize = meshData.vertexData.size();
        fwrite(&vertexDataSize, sizeof(vertexDataSize), 1, f);
        if (vertexDataSize > 0)
            fwrite(meshData.vertexData.data(), 1, vertexDataSize, f);

        uint64_t indexDataSize = meshData.indexData.size();
        fwrite(&indexDataSize, sizeof(indexDataSize), 1, f);
        if (indexDataSize > 0)
            fwrite(meshData.indexData.data(), sizeof(uint32_t), indexDataSize, f);

        saveMaterialList(meshData.materials);
        saveStringList(meshData.diffuseTextureFiles);
        saveStringList(meshData.emissiveTextureFiles);
        saveStringList(meshData.normalTextureFiles);
        saveStringList(meshData.occlusionTextureFiles);

        fclose(f);
        printf("MeshData saved to %s\n", path.c_str());
    }

    bool loadMeshData(const std::string& path, MeshData& meshData) {
        FILE* f = fopen(path.c_str(), "rb");
        if (!f) return false;

        auto loadString = [](FILE* f) -> std::string {
            uint64_t len = 0;
            fread(&len, sizeof(len), 1, f);
            std::string s(len, '\0');
            fread(&s[0], 1, len, f);
            return s;
            };

        auto loadMaterialList = [&](std::vector<Material>& materials) {
            uint64_t count = 0;
            fread(&count, sizeof(count), 1, f);
            materials.resize(count);
            for (auto& m : materials) {
                fread(&m.emissiveFactor, sizeof(glm::vec4), 1, f);
                fread(&m.baseColorFactor, sizeof(glm::vec4), 1, f);
                fread(&m.roughness, sizeof(float), 1, f);
                fread(&m.metallicFactor, sizeof(float), 1, f);
                fread(&m.alphaTest, sizeof(float), 1, f);
                fread(&m.transparencyFactor, sizeof(float), 1, f);
                fread(&m.baseColorTexture, sizeof(uint32_t), 1, f);
                fread(&m.specularTexture, sizeof(uint32_t), 1, f);
                fread(&m.emissiveTexture, sizeof(uint32_t), 1, f);
                fread(&m.normalTexture, sizeof(uint32_t), 1, f);
                fread(&m.opacityTexture, sizeof(uint32_t), 1, f);
                fread(&m.occlusionTexture, sizeof(uint32_t), 1, f);
                m.baseColorTexturePath = loadString(f);
                m.specularTexturePath = loadString(f);
                m.emissiveTexturePath = loadString(f);
                m.normalTexturePath = loadString(f);
                m.opacityTexturePath = loadString(f);
                m.occlusionTexturePath = loadString(f);
            }
            };

        auto loadStringList = [&](std::vector<std::string>& list) {
            uint64_t count;
            fread(&count, sizeof(count), 1, f);
            list.resize(count);
            for (uint64_t i = 0; i < count; ++i)
                list[i] = loadString(f);
            };

        uint64_t numMeshes = 0;
        fread(&numMeshes, sizeof(numMeshes), 1, f);
        meshData.meshes.resize(numMeshes);
        if (numMeshes > 0)
            fread(meshData.meshes.data(), sizeof(Mesh), numMeshes, f);

        uint64_t vertexDataSize = 0;
        fread(&vertexDataSize, sizeof(vertexDataSize), 1, f);
        meshData.vertexData.resize(vertexDataSize);
        if (vertexDataSize > 0)
            fread(meshData.vertexData.data(), 1, vertexDataSize, f);

        uint64_t indexDataSize = 0;
        fread(&indexDataSize, sizeof(indexDataSize), 1, f);
        meshData.indexData.resize(indexDataSize);
        if (indexDataSize > 0)
            fread(meshData.indexData.data(), sizeof(uint32_t), indexDataSize, f);

        loadMaterialList(meshData.materials);
        loadStringList(meshData.diffuseTextureFiles);
        loadStringList(meshData.emissiveTextureFiles);
        loadStringList(meshData.normalTextureFiles);
        loadStringList(meshData.occlusionTextureFiles);

        fclose(f);
        printf("MeshData loaded from %s\n", path.c_str());
        return true;
    }


}




