#include "tools.h"

namespace LZ::Tools {

    glm::vec3 Tools::faceCoordsToXYZ(int i, int j, int faceID, int faceSize) {
        float u = (2.0f * (i + 0.5f) / faceSize) - 1.0f;
        float v = (2.0f * (j + 0.5f) / faceSize) - 1.0f;

        switch (faceID) {
        case 0: return glm::normalize(glm::vec3(1.0f, -v, -u)); // +X
        case 1: return glm::normalize(glm::vec3(-1.0f, -v, u)); // -X
        case 2: return glm::normalize(glm::vec3(u, 1.0f, v)); // +Y
        case 3: return glm::normalize(glm::vec3(u, -1.0f, -v)); // -Y
        case 4: return glm::normalize(glm::vec3(u, -v, 1.0f)); // +Z
        case 5: return glm::normalize(glm::vec3(-u, -v, -1.0f)); // -Z
        default: return glm::vec3(0.0f);
        }
    }

    Bitmap Tools::convertEquirectangularToCubemapFaces(const Bitmap& equirect) {
        
        if (equirect.mFormat != BitmapFormat::Float) {
            throw std::runtime_error("input must be float format");
        }

        int faceSize = equirect.mWidth / 4;
        Bitmap cubemap(faceSize, faceSize, 6, equirect.mChannels, equirect.mFormat);

        int clampW = equirect.mWidth - 1;
        int clampH = equirect.mHeight - 1;

        for (int face = 0; face < 6; ++face) {
            for (int i = 0; i < faceSize; ++i) {
                for (int j = 0; j < faceSize; ++j) {
                    glm::vec3 P = faceCoordsToXYZ(i, j, face, faceSize);
                    float R = std::hypot(P.x, P.y);
                    float theta = std::atan2(P.y, P.x);
                    float phi = std::atan2(P.z, R);

                    float Uf = 2.0f * faceSize * (theta + glm::pi<float>()) / glm::pi<float>();
                    float Vf = 2.0f * faceSize * (glm::half_pi<float>() - phi) / glm::pi<float>();

                    int U1 = std::clamp(int(std::floor(Uf)), 0, clampW);
                    int V1 = std::clamp(int(std::floor(Vf)), 0, clampH);
                    int U2 = std::clamp(U1 + 1, 0, clampW);
                    int V2 = std::clamp(V1 + 1, 0, clampH);

                    float s = Uf - U1;
                    float t = Vf - V1;

                    glm::vec4 A = equirect.getPixelFloat(U1, V1);
                    glm::vec4 B = equirect.getPixelFloat(U2, V1);
                    glm::vec4 C = equirect.getPixelFloat(U1, V2);
                    glm::vec4 D = equirect.getPixelFloat(U2, V2);

                    glm::vec4 color = A * (1 - s) * (1 - t) + B * s * (1 - t) + C * (1 - s) * t + D * s * t;

                    cubemap.setPixelFloat(i, j, face, color); 
                }
            }
        }

        return cubemap;
    }
}