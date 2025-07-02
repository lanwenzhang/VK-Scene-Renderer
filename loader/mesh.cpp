#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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

        meshData.diffuseTextureFiles.push_back("dummy.png");
        std::unordered_map<std::string, int> diffuseTextureMap;
        diffuseTextureMap["dummy.png"] = 0;

        Assimp::Importer importer;

        const aiScene* aiScene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_JoinIdenticalVertices
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
                std::string uri = str.C_Str();
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


            // ---------- other texture types ----------
            struct TexTypeInfo {
                aiTextureType type;
                const char* name;
            };

            std::vector<TexTypeInfo> texTypes = {
                { aiTextureType_SPECULAR,   "Specular" },
                { aiTextureType_NORMALS,    "Normals" },
                { aiTextureType_HEIGHT,     "Height/Bump" },
                { aiTextureType_EMISSIVE,   "Emissive" },
                { aiTextureType_OPACITY,    "Opacity" },
                { aiTextureType_LIGHTMAP,   "Lightmap/Occlusion" },
                { aiTextureType_METALNESS,  "Metalness" },
                { aiTextureType_DIFFUSE_ROUGHNESS, "Roughness" },
                { aiTextureType_AMBIENT_OCCLUSION, "AO" }
            };

            for (auto& t : texTypes) {
                if (AI_SUCCESS == aiMat->GetTexture(t.type, 0, &str)) {
                    std::string uri = str.C_Str();
                    std::cout << "[Material " << i << "] "
                        << t.name << " texture: " << uri << std::endl;
                }
            }

            meshData.materials.push_back(m);
            scene.materialNames.push_back("Material_" + std::to_string(i));

            std::cout << "Material " << i
                << " -> baseColorTex = " << m.baseColorTexture
                << std::endl;
        }


        // ========== LOAD MESHES + SCENE NODES ==========

        int root = addNode(scene, -1, 0);
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
        glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
        glm::mat4 modelTransform = rotateMat * scaleMat;

        scene.localTransform[root] = modelTransform;
        scene.globalTransform[root] = modelTransform;
        scene.nodeNames.push_back("Root");
        scene.nameForNode[root] = uint32_t(scene.nodeNames.size() - 1);

        size_t vertexStart = 0;

        std::function<void(aiNode*, int, int)> traverse;
        traverse = [&](aiNode* node, int parent, int level) {
            std::cout
                << std::string(level * 2, ' ')
                << "[Node] " << node->mName.C_Str()
                << ", meshes: " << node->mNumMeshes
                << ", children: " << node->mNumChildren
                << std::endl;

            aiMatrix4x4 m = node->mTransformation;
            std::cout << std::string(level * 2, ' ') << "  Transform matrix:\n"
                << std::string(level * 2, ' ')
                << m.a1 << " " << m.a2 << " " << m.a3 << " " << m.a4 << "\n"
                << std::string(level * 2, ' ')
                << m.b1 << " " << m.b2 << " " << m.b3 << " " << m.b4 << "\n"
                << std::string(level * 2, ' ')
                << m.c1 << " " << m.c2 << " " << m.c3 << " " << m.c4 << "\n"
                << std::string(level * 2, ' ')
                << m.d1 << " " << m.d2 << " " << m.d3 << " " << m.d4 << "\n";

            int nodeId = addNode(scene, parent, level);
            scene.nodeNames.push_back(node->mName.C_Str());
            scene.nameForNode[nodeId] = uint32_t(scene.nodeNames.size() - 1);

            glm::mat4 local(1.0f);
            local = glm::transpose(glm::make_mat4(&m.a1));
            scene.localTransform[nodeId] = local;
            scene.globalTransform[nodeId] = glm::mat4(1.0f);

            for (unsigned int meshIndex = 0; meshIndex < node->mNumMeshes; meshIndex++) {
                aiMesh* aiMesh = aiScene->mMeshes[node->mMeshes[meshIndex]];

                std::cout
                    << std::string(level * 2, ' ')
                    << "  [Mesh] Index: " << meshIndex
                    << ", vertices: " << aiMesh->mNumVertices
                    << ", faces: " << aiMesh->mNumFaces
                    << ", material index: " << aiMesh->mMaterialIndex
                    << std::endl;

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

                    float vertex[5] = { pos.x, pos.y, pos.z, uv.x, uv.y };
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

                DrawData dd;
                dd.transformId = nodeId;
                dd.materialId = meshData.materials[m.materialID].baseColorTexture;
                scene.drawDataArray.push_back(dd);
            }

            if (node->mNumMeshes > 0) {
                scene.meshForNode[nodeId] = uint32_t(meshData.meshes.size() - node->mNumMeshes);
            }
            for (unsigned int i = 0; i < node->mNumChildren; i++) {
                traverse(node->mChildren[i], nodeId, level + 1);
            }
            };

        traverse(aiScene->mRootNode, root, 1);

        recalculateGlobalTransforms(scene);

        std::cout << "Loaded mesh (Assimp): " << path
            << ", nodes: " << scene.hierarchy.size()
            << ", meshes: " << meshData.meshes.size()
            << ", vertices: " << vertexStart << std::endl;

        return true;
    }


}


