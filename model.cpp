#include "model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace FF {

	void Model::loadModel(const std::string& path, const Wrapper::Device::Ptr& device) {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || !scene->HasMeshes()) {
            throw std::runtime_error("Error: failed to load model with Assimp");
        }

        mPositions.clear();
        mUVs.clear();
        mIndexDatas.clear();

        const aiMesh* mesh = scene->mMeshes[0];

        for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
    
            mPositions.push_back(mesh->mVertices[i].x);
            mPositions.push_back(mesh->mVertices[i].y);
            mPositions.push_back(mesh->mVertices[i].z);

   
            if (mesh->HasTextureCoords(0)) {
                mUVs.push_back(mesh->mTextureCoords[0][i].x);
                mUVs.push_back(mesh->mTextureCoords[0][i].y);
            }
            else {
                mUVs.push_back(0.0f);
                mUVs.push_back(0.0f);
            }
        }

     
        for (uint32_t f = 0; f < mesh->mNumFaces; f++) {
            const aiFace& face = mesh->mFaces[f];
 
            if (face.mNumIndices != 3) continue;

            for (uint32_t j = 0; j < 3; j++) {
                mIndexDatas.push_back(face.mIndices[j]);
            }
        }

		mPositionBuffer = Wrapper::Buffer::createVertexBuffer(device, mPositions.size() * sizeof(float), mPositions.data());

		mUVBuffer = Wrapper::Buffer::createVertexBuffer(device, mUVs.size() * sizeof(float), mUVs.data());

		mIndexBuffer = Wrapper::Buffer::createIndexBuffer(device, mIndexDatas.size() * sizeof(float), mIndexDatas.data());
	}
}