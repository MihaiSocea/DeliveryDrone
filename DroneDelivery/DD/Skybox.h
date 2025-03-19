#pragma once

#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include "core/gpu/mesh.h"
#include "core/gpu/shader.h"
#include "core/gpu/texture2D.h"

class Skybox {
public:
    Skybox();
    ~Skybox();

    void LoadTextures(const std::vector<std::string>& texturePaths);
    void Render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

private:
    unsigned int cubemapTextureID;
    Mesh* cubeMesh;
    Shader* shader;

    unsigned int LoadCubemap(const std::vector<std::string>& texturePaths);
};
