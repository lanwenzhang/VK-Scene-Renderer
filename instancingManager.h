#pragma once

#include "base.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/buffer.h"


namespace FF {

	class InstancingManager {
	public:

		using Ptr = std::shared_ptr<InstancingManager>;
		static Ptr create() { return std::make_shared<InstancingManager>(); }

		InstancingManager();
		~InstancingManager();

		bool init(const Wrapper::Device::Ptr& device, uint32_t instancingCount, uint32_t frameCount);

		[[nodiscard]] auto getBufferDeviceAddress_PosAngle() const {
			if (!mBufferPosAngle) {
				throw std::runtime_error("mBufferPosAngle is nullptr");
			}

			return mBufferPosAngle->getDeviceAddress();

		}
		[[nodiscard]] auto getBufferDeviceAddress_Matrix(uint32_t frameIndex) const {
			if (frameIndex >= mBufferMatrices.size()) {
				throw std::runtime_error("Frame index out of range in getBufferDeviceAddress_Matrix");
			}
			if (!mBufferMatrices[frameIndex]) {
				throw std::runtime_error("Matrix buffer is nullptr for given frame");
			}

			return mBufferMatrices[frameIndex]->getDeviceAddress();
		}

		[[nodiscard]] auto& getInstancingData() const { return mPositionsAngles; }
		[[nodiscard]] auto getInstancingCount() const { return mInstancingCount; }
		[[nodiscard]] auto getBufferModelMatrix(uint32_t frameIndex) const {

			if (frameIndex >= mBufferMatrices.size()) {
				throw std::runtime_error("Invalid frame index in getBufferModelMatrix");
			}

			return mBufferMatrices[frameIndex];
		}

	private:

		Wrapper::Device::Ptr mDevice{ nullptr };
		uint32_t mInstancingCount{ 0 };
		uint32_t mFrameCount{ 0 };

		std::vector<glm::vec4> mPositionsAngles{};
		Wrapper::Buffer::Ptr mBufferPosAngle{ nullptr };
		std::vector<Wrapper::Buffer::Ptr> mBufferMatrices{};
	};
}

