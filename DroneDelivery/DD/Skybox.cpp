#include "Skybox.h"
#include <stb/stb_image.h>
#include <iostream>
#include "core/gpu/mesh.h"
#include "core/gpu/shader.h"
#include <core/managers/resource_path.h>

Skybox::Skybox() {
    cubeMesh = new Mesh("cube");
    cubeMesh->LoadMesh(RESOURCE_PATH::MODELS + "/primitives", "box.obj");

    shader = new Shader("SkyboxShader");
    shader->AddShader("shaders/skybox_vertex.glsl", GL_VERTEX_SHADER);
    shader->AddShader("shaders/skybox_fragment.glsl", GL_FRAGMENT_SHADER);
    shader->CreateAndLink();
}

Skybox::~Skybox() {
    delete cubeMesh;
    delete shader;
}

void Skybox::LoadTextures(const std::vector<std::string>& texturePaths) {
    cubemapTextureID = LoadCubemap(texturePaths);
}

void Skybox::Render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    glDepthFunc(GL_LEQUAL);

    shader->Use();
    glm::mat4 view = glm::mat4(glm::mat3(viewMatrix)); // Remove translation from the view matrix
    glUniformMatrix4fv(shader->GetUniformLocation("view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(shader->GetUniformLocation("projection"), 1, GL_FALSE, &projectionMatrix[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
    cubeMesh->Render();

    glDepthFunc(GL_LESS);
}

unsigned int Skybox::LoadCubemap(const std::vector<std::string>& texturePaths) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < texturePaths.size(); i++) {
        unsigned char* data = stbi_load(texturePaths[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cerr << "Failed to load cubemap texture at path: " << texturePaths[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
