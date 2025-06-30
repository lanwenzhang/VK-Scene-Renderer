#pragma once

#include "../common.h"

namespace LZ::Tools {

    enum BitmapFormat {
        UnsignedByte,
        Float
    };

    struct Bitmap {
        int mWidth = 0;
        int mHeight = 0;
        int mDepth = 1;
        int mChannels = 3;
        BitmapFormat mFormat = UnsignedByte;
        std::vector<uint8_t> mData;

        Bitmap() = default;

        Bitmap(int width, int height, int channels, BitmapFormat format)
            : mWidth(width), mHeight(height), mDepth(1), mChannels(channels), mFormat(format),
            mData(width* height* channels* getBytesPerChannel(format)) {
        }

        Bitmap(int width, int height, int depth, int channels, BitmapFormat format)
            : mWidth(width), mHeight(height), mDepth(depth), mChannels(channels), mFormat(format),
            mData(width* height* depth* channels* getBytesPerChannel(format)) {
        }

        Bitmap(int width, int height, int channels, BitmapFormat format, const void* ptr)
            : mWidth(width), mHeight(height), mDepth(1), mChannels(channels), mFormat(format),
            mData(width* height* channels* getBytesPerChannel(format)) {
            std::memcpy(mData.data(), ptr, mData.size());
        }

        static int getBytesPerChannel(BitmapFormat format) {
            switch (format) {
            case UnsignedByte: return 1;
            case Float: return 4;
            default: return 0;
            }
        }

        glm::vec4 getPixelFloat(int x, int y, int z = 0) const {
            const int offset = mChannels * ((z * mHeight + y) * mWidth + x);
            const float* data = reinterpret_cast<const float*>(mData.data());
            return glm::vec4(
                mChannels > 0 ? data[offset + 0] : 0.0f,
                mChannels > 1 ? data[offset + 1] : 0.0f,
                mChannels > 2 ? data[offset + 2] : 0.0f,
                mChannels > 3 ? data[offset + 3] : 0.0f);
        }

        void setPixelFloat(int x, int y, int z, const glm::vec4& color) {
            const int offset = mChannels * ((z * mHeight + y) * mWidth + x);
            float* data = reinterpret_cast<float*>(mData.data());
            if (mChannels > 0) data[offset + 0] = color.x;
            if (mChannels > 1) data[offset + 1] = color.y;
            if (mChannels > 2) data[offset + 2] = color.z;
            if (mChannels > 3) data[offset + 3] = color.w;
        }
    };

}


