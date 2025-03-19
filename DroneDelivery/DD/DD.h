#pragma once
#include "components/simple_scene.h"
#include "DD_camera.h"
#include "DroneDelivery/DD/Skybox.h"
namespace m1 {
    class Tema2 : public gfxc::SimpleScene {
    public:
        struct ViewportArea {
            ViewportArea() : x(0), y(0), width(1), height(1) {}
            ViewportArea(int x, int y, int width, int height)
                : x(x), y(y), width(width), height(height) {}
            int x, y, width, height;
        };

        struct Obstacle {
            glm::vec3 position;
            glm::vec3 scale;
            float coneBaseRadius;
            float coneHeight;
        };

        Tema2();
        ~Tema2();

        void LoadMesh(const std::string& meshName, const std::string& fileName);

        void Init() override;
        Mesh* CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned short>& indices);
    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void RenderDestinationOnMinimap();
        void FrameEnd() override;
        void RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix) override;
        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;
        void GenerateBuildings();
        void RenderBuildings();
        void GenerateTerrain();
        void RenderTerrain();
        void Render(float deltaTimeSeconds);
        void RenderMinimap(float deltaTimeSeconds);
        void GenerateObstacles();
        void RenderClouds();
        void RenderObstacles();
        Mesh* GenerateCylinder(const std::string& name, int numSides, float height, float radius);
        Mesh* GenerateCone(const std::string& name, int numSides, float height, float radius);
        void RenderPackageAndDestination();
        bool CheckCollision(const glm::vec3& objectPosition, float objectSize, const glm::vec3& targetPosition, float targetSize);
        glm::vec3 GenerateRandomPosition(float range, float minDistance);
        void GeneratePackageAndDestination();
        void RenderMMesh(Mesh* mesh, Shader* shader, glm::mat4 modelMatrix, glm::mat4 projectionMatrix);
        bool CheckCollision(const glm::vec3& position);
        void RenderArrow();
        void RenderDrone();
        void UpdateCamera();
        void RenderMinimap();
        void SendUniforms(int cam);
        bool isJumping;
        float jumpVelocity;
        static constexpr float SCALE_FACTOR = .004f;
        int score = 0;
        float bladeRotation = 0;
        bool packagePicked = false;
        float translateX, translateY, translateZ;
        float movementOfClouds;
        float scaleX, scaleY, scaleZ;
        float angularStepOX, angularStepOY, angularStepOZ;
        float bladeRotationSpeed;
        Skybox* skybox;
        glm::mat4 minimapProjectionMatrix;
        glm::vec3 dronePosition;
        glm::vec3 cameraPosition;

        GLenum polygonMode;
        ViewportArea miniViewportArea;
        glm::vec3 packagePosition;
        glm::vec3 destinationPosition;
        std::vector<Obstacle> obstacles;
        std::vector<Obstacle> buildings;
        float positionCloudsX[10];
        float positionCloudsY[10];
        float rotationsCloudsOX[10];
        float rotationsCloudsOY[10];
        float rotationsCloudsOZ[10];
    protected:
        implemented::Camera* camera, * miniCamera;
        glm::mat4 projectionMatrix;
        bool renderCameraTarget;
        glm::mat4 modelMatrix;
        // TODO(student): If you need any other class variables, define them here.
        bool orthoProjection;
        float projectionFov, projectionWidth, projectionHeight;

    };
}   // namespace m1