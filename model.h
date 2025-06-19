#pragma once
#include "base.h"
#include "vulkanWrapper/buffer.h"


namespace FF {

	class Model {
	public:

		using Ptr = std::shared_ptr<Model>;
		static Ptr create(const Wrapper::Device::Ptr& device) { return std::make_shared<Model>(device); }

		Model(const Wrapper::Device::Ptr& device) {

			mUniform.mModelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		}

		~Model() {}

		void loadModel(const std::string& path, const Wrapper::Device::Ptr& device);

		// Binding description
		std::vector<VkVertexInputBindingDescription> getVertexInputBindingDescriptions() {

			std::vector<VkVertexInputBindingDescription> bindingDes{};
			bindingDes.resize(2);

			bindingDes[0].binding = 0;
			bindingDes[0].stride = sizeof(float) * 3;
			bindingDes[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			bindingDes[1].binding = 1;
			bindingDes[1].stride = sizeof(float) * 2;
			bindingDes[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDes;
		}

		// Attribute description
		std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {

			std::vector<VkVertexInputAttributeDescription> attributeDes{};
			attributeDes.resize(2);

			attributeDes[0].binding = 0;
			attributeDes[0].location = 0;
			attributeDes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDes[0].offset = 0;

			attributeDes[1].binding = 1;
			attributeDes[1].location = 2;
			attributeDes[1].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDes[1].offset = 0;

			return attributeDes;
		}

		[[nodiscard]] auto getVertexBuffers() const {
			
			std::vector<VkBuffer> buffers{ mPositionBuffer->getBuffer(), mUVBuffer->getBuffer()};

			return buffers;
		}

		[[nodiscard]] auto getIndexBuffer() const { return mIndexBuffer; }

		[[nodiscard]] auto getIndexCount() const { return mIndexDatas.size(); }

		[[nodiscard]] auto getUniform() const { return mUniform; }

		void setModelMatrix(const glm::mat4 matrix) { mUniform.mModelMatrix = matrix; }

		void update() {

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::translate(model, glm::vec3(0.0f, 4.0f, 0.0f));
			model = glm::rotate(model, glm::radians(mAngle), glm::vec3(0.0f, 0.0f, 1.0f));
			mUniform.mModelMatrix = model;

			mAngle += 0.01f;
		}

	private:

		std::vector<float> mPositions{};
		std::vector<float> mUVs{};
		std::vector<unsigned int> mIndexDatas{};
		
		Wrapper::Buffer::Ptr mPositionBuffer{ nullptr };
		Wrapper::Buffer::Ptr mUVBuffer{ nullptr };
		Wrapper::Buffer::Ptr mIndexBuffer{ nullptr };

		ObjectUniform mUniform;
		float mAngle{ 0.0f };
	};

}