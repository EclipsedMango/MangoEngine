
#ifndef MANGORENDERING_IBLPRECOMPUTER_H
#define MANGORENDERING_IBLPRECOMPUTER_H

#include <memory>
#include "Renderer/Materials/Texture.h"
#include "Renderer/Shader.h"
#include "Renderer/Meshes/Mesh.h"

class Texture;

class IBLPrecomputer {
public:
    struct Result {
        std::unique_ptr<Texture> irradiance; // 32x32 cubemap
        std::unique_ptr<Texture> prefiltered; // 128x128 cubemap w/ mips
        std::unique_ptr<Texture> brdfLut; // 512x512 2D RG16F
    };

    static Result Compute(const Texture& envCubemap);

    static constexpr int IRRADIANCE_SIZE = 32;
    static constexpr int PREFILTER_SIZE = 128;
    static constexpr int PREFILTER_MIP_LEVELS = 5;
    static constexpr int BRDF_LUT_SIZE = 512;
private:
    static std::unique_ptr<Texture> ComputeIrradiance(const Texture& env, GLuint captureFbo, GLuint captureRbo, const Shader& shader, const Mesh& cube);
    static std::unique_ptr<Texture> ComputePrefiltered(const Texture& env, GLuint captureFbo, GLuint captureRbo, const Shader& shader, const Mesh& cube);
    static std::unique_ptr<Texture> ComputeBrdfLut(GLuint captureFbo, GLuint captureRbo, const Shader& shader);
};


#endif //MANGORENDERING_IBLPRECOMPUTER_H