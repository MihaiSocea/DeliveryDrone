#include "DroneDelivery/DD/DD.h"
#include <glm/gtc/random.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <lab_m1/lab4/transform3D.h>

using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema2::Tema2()
{
}


Tema2::~Tema2()
{
}


void Tema2::Init()
{
    glm::vec3 packagePosition;
    glm::vec3 destinationPosition;
    bool packagePicked = false;
    translateX = 0;
    translateY = 10;
    translateZ = 0;
    scaleX = 1;
    scaleY = 1;
    scaleZ = 1;
    angularStepOX = 0;
    angularStepOY = 0;
    angularStepOZ = 0;
    bladeRotationSpeed = 3.0f;
    renderCameraTarget = false;
    orthoProjection = false;
    projectionFov = 60.0f;
    projectionWidth = 16.0f;
    projectionHeight = 9.0f;
    GenerateTerrain();
    GenerateObstacles();
    GenerateBuildings();
    GeneratePackageAndDestination();
    camera = new implemented::Camera();
    miniCamera = new implemented::Camera();
    meshes["arrowBody"] = GenerateCylinder("arrowBody", 20, 2.0f, 0.1f);
    meshes["arrowHead"] = GenerateCone("arrowHead", 20, 0.5f, 0.2f);
    glm::ivec2 resolution = window->GetResolution();
    miniViewportArea = ViewportArea(50, 50, resolution.x / 5.f, resolution.y / 5.f);
    {
        Mesh* mesh = new Mesh("box");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* cube = new Mesh("cube");
        cube->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[cube->GetMeshID()] = cube;
    }
    {
        Mesh* body = new Mesh("body");
        body->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[body->GetMeshID()] = body;

        Mesh* blade = new Mesh("blade");
        blade->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[blade->GetMeshID()] = blade;
    }
    for (int i = 0; i < 10; i++) {
        Mesh* cloud = new Mesh("cloud" + std::to_string(i));
        cloud->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[cloud->GetMeshID()] = cloud;
    }
}


void Tema2::FrameStart()
{

    projectionMatrix = orthoProjection
        ? glm::ortho(-projectionWidth / 2, projectionWidth / 2, -projectionHeight / 2, projectionHeight / 2, .01f, 200.0f)
        : glm::perspective(RADIANS(projectionFov), window->props.aspectRatio, 0.01f, 200.0f);
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


}
bool Tema2::CheckCollision(const glm::vec3& newPosition) {
    float droneSizeX = 2.0f;
    float droneSizeY = 0.25f;
    for (const auto& obstacle : obstacles) {
        glm::vec3 obstacleMin = obstacle.position - obstacle.scale * 0.5f;
        glm::vec3 obstacleMax = obstacle.position + obstacle.scale * 0.5f;

        if (newPosition.x + droneSizeX > obstacleMin.x &&
            newPosition.x - droneSizeX < obstacleMax.x &&
            newPosition.y + droneSizeY > obstacleMin.y &&
            newPosition.y - droneSizeY < obstacleMax.y + 10.0f &&
            newPosition.z + droneSizeX > obstacleMin.z &&
            newPosition.z - droneSizeX < obstacleMax.z) {
            return true;
        }
    }

    for (const auto& building : buildings) {
        glm::vec3 buildingMin = building.position - building.scale * 0.5f;
        glm::vec3 buildingMax = building.position + building.scale * 0.5f;

        if (newPosition.x + droneSizeX > buildingMin.x &&
            newPosition.x - droneSizeX < buildingMax.x &&
            newPosition.y + droneSizeY > buildingMin.y &&
            newPosition.y - droneSizeY < buildingMax.y &&
            newPosition.z + droneSizeX > buildingMin.z &&
            newPosition.z - droneSizeX < buildingMax.z) {
            return true;
        }
    }
    if (newPosition.y < 0) {
        return true;
    }

    return false;
}
void Tema2::RenderArrow() {
    glm::mat4 modelMatrix = glm::mat4(1);
    glm::vec3 arrowPosition = glm::vec3(translateX, translateY, translateZ);

    glm::mat4 coneMatrix = glm::mat4(1);
    coneMatrix = glm::translate(modelMatrix, arrowPosition);
    coneMatrix = glm::scale(coneMatrix, glm::vec3(3.0f, 5.0f, 5.0f));
    RenderMesh(meshes["arrowHead"], shaders["Simple"], coneMatrix);
    glm::mat4 cylinderMatrix = glm::mat4(1);
    cylinderMatrix = glm::translate(modelMatrix, arrowPosition);
    cylinderMatrix = glm::scale(cylinderMatrix, glm::vec3(10.0f, 10.0f, 10.0f));
}

void Tema2::RenderDrone()
{
    glm::mat4 modelMatrix = glm::mat4(1);
    glm::vec3 dronePosition = glm::vec3(translateX, translateY, translateZ);
    float xRotationAngle = glm::radians(45.0f);
    modelMatrix = glm::mat4(1);
    modelMatrix *= transform3D::Translate(translateX, translateY, translateZ);
    modelMatrix *= transform3D::RotateOY(angularStepOY);
    modelMatrix *= transform3D::RotateOY(xRotationAngle);
    modelMatrix *= transform3D::Scale(4.0f, 0.2f, 0.5f);
    RenderMesh(meshes["body"], shaders["Tema2"], modelMatrix);
    modelMatrix = glm::mat4(1);
    modelMatrix *= transform3D::Translate(translateX, translateY, translateZ);
    modelMatrix *= transform3D::RotateOY(angularStepOY);
    modelMatrix *= transform3D::RotateOY(-xRotationAngle);
    modelMatrix *= transform3D::Scale(4.0f, 0.2f, 0.5f);

    RenderMesh(meshes["body"], shaders["Tema2"], modelMatrix);

    float armLength = 1.75f;
    float cubeSize = 0.5f;
    float cubeHeight = 0.25f;
    glm::vec3 cornerOffsets[4] = {
        glm::vec3(armLength / sqrt(2.0f), cubeHeight, armLength / sqrt(2.0f)),
        glm::vec3(-armLength / sqrt(2.0f), cubeHeight, armLength / sqrt(2.0f)),
        glm::vec3(-armLength / sqrt(2.0f), cubeHeight, -armLength / sqrt(2.0f)),
        glm::vec3(armLength / sqrt(2.0f), cubeHeight, -armLength / sqrt(2.0f))
    };

    float cornerAngles[4] = { 45.0f, -45.0f, -135.0f, 135.0f };

    for (int i = 0; i < 4; i++)
    {
        modelMatrix = glm::mat4(1);
        modelMatrix *= transform3D::Translate(translateX, translateY, translateZ);
        modelMatrix *= transform3D::RotateOY(angularStepOY);
        modelMatrix *= transform3D::Translate(cornerOffsets[i]);
        modelMatrix *= transform3D::RotateOY(glm::radians(cornerAngles[i]));
        modelMatrix *= transform3D::Scale(cubeSize, cubeSize, cubeSize);
        RenderMesh(meshes["body"], shaders["Tema2"], modelMatrix);
    }

    float bladeOffsetY = cubeHeight + 0.5f * cubeSize;
    float bladeSize = 0.5f;

    for (int i = 0; i < 4; i++)
    {
        modelMatrix = glm::mat4(1);
        modelMatrix *= transform3D::Translate(translateX, translateY, translateZ);
        modelMatrix *= transform3D::RotateOY(angularStepOY);
        modelMatrix *= transform3D::Translate(cornerOffsets[i]);
        modelMatrix *= transform3D::Translate(0, bladeOffsetY, 0);
        modelMatrix *= transform3D::RotateOY(glm::radians(cornerAngles[i]));
        modelMatrix *= transform3D::RotateOY(bladeRotation);
        modelMatrix *= transform3D::Scale(bladeSize, 0.1f, 2.0f);
        RenderMesh(meshes["blade"], shaders["Simple"], modelMatrix);
    }
}
Mesh* Tema2::GenerateCone(const std::string& name, int numSides, float height, float radius) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    vertices.emplace_back(glm::vec3(0, height, 0), glm::vec3(0, 1, 0), glm::vec3(0.1f, 0.8f, 0.1f));

    for (int i = 0; i < numSides; ++i) {
        float angle = (float)i / numSides * glm::two_pi<float>();
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.emplace_back(glm::vec3(x, 0, z), glm::vec3(0, -1, 0), glm::vec3(0.1f, 0.8f, 0.1f));
    }

    vertices.emplace_back(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0.1f, 0.8f, 0.1f));

    for (int i = 0; i < numSides; ++i) {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back((i + 1) % numSides + 1);
    }

    int centerIndex = numSides + 1;
    for (int i = 0; i < numSides; ++i) {
        indices.push_back(centerIndex);
        indices.push_back((i + 1) % numSides + 1);
        indices.push_back(i + 1);
    }
    Mesh* cone = new Mesh(name);
    cone->InitFromData(vertices, indices);
    return cone;
}
Mesh* Tema2::GenerateCylinder(const std::string& name, int numSides, float height, float radius) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;
    for (int i = 0; i <= numSides; ++i) {
        float angle = (float)i / numSides * glm::two_pi<float>();
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        vertices.emplace_back(glm::vec3(x, height / 2, z), glm::vec3(0, 1, 0), glm::vec3(0.8f, 0.3f, 0.3f));
        vertices.emplace_back(glm::vec3(x, 0, z), glm::vec3(0, -1, 0), glm::vec3(0.8f, 0.3f, 0.3f));
    }

    vertices.emplace_back(glm::vec3(0, height / 2, 0), glm::vec3(0, 1, 0), glm::vec3(0.8f, 0.3f, 0.3f));
    vertices.emplace_back(glm::vec3(0, -height / 2, 0), glm::vec3(0, -1, 0), glm::vec3(0.8f, 0.3f, 0.3f));
    int topCenterIndex = numSides * 2;
    int bottomCenterIndex = topCenterIndex + 1;
    for (int i = 0; i < numSides; ++i) {
        indices.push_back(topCenterIndex);
        indices.push_back(i * 2);
        indices.push_back((i * 2 + 2) % (numSides * 2));

        indices.push_back(bottomCenterIndex);
        indices.push_back((i * 2 + 3) % (numSides * 2));
        indices.push_back(i * 2 + 1);
    }
    for (int i = 0; i < numSides; ++i) {
        int top1 = i * 2;
        int top2 = (i * 2 + 2) % (numSides * 2);
        int bottom1 = top1 + 1;
        int bottom2 = top2 + 1;

        indices.push_back(top1);
        indices.push_back(bottom1);
        indices.push_back(top2);

        indices.push_back(top2);
        indices.push_back(bottom1);
        indices.push_back(bottom2);
    }
    Mesh* cylinder = new Mesh(name);
    cylinder->InitFromData(vertices, indices);
    return cylinder;
}
void Tema2::GenerateBuildings() {
    buildings.clear();

    int numBuildings = 15; 
    float terrainRange = 50.0f;
    float minDistance = 5.0f;

    for (int i = 0; i < numBuildings; ++i) {
        glm::vec3 position;
        glm::vec3 scale;
        bool validPosition = false;

        while (!validPosition) {
            position.x = glm::linearRand(-terrainRange / 2.0f, terrainRange / 2.0f);
            position.z = glm::linearRand(-terrainRange / 2.0f, terrainRange / 2.0f);
            position.y = 0.0f;

            scale = glm::vec3(
                glm::linearRand(5.0f, 10.0f),
                glm::linearRand(3.0f, 4.0f),
                glm::linearRand(2.0f, 4.0f)
            );

            validPosition = true;
            if (position.x < 10.0f) {
                validPosition = false;
            }
            for (const auto& obstacle : obstacles) {
                float distance = glm::distance(glm::vec2(position.x, position.z),
                    glm::vec2(obstacle.position.x, obstacle.position.z));
                float distanceFromDrone = glm::distance(glm::vec2(position.x, position.z),
                    glm::vec2(0, 0));
                if (distance < minDistance || distanceFromDrone < minDistance) {
                    validPosition = false;
                    break;
                }
            }
        }

        // Add the building if a valid position is found
        buildings.push_back({ position, scale });

        std::string buildingName = "building_" + std::to_string(i);
        meshes[buildingName] = meshes["cube"];
    }
}



void Tema2::RenderBuildings() {
    for (int i = 0; i < buildings.size(); ++i) {
        const Obstacle& building = buildings[i];
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, building.position);
        modelMatrix = glm::scale(modelMatrix, building.scale);
        glUseProgram(shaders["Tema2"]->GetProgramID());
        int colorLocation = glGetUniformLocation(shaders["Tema2"]->GetProgramID(), "object_color");
        glUniform3f(colorLocation, 0.5f, 0.5f, 0.5f);

        std::string buildingName = "building_" + std::to_string(i);
        RenderMesh(meshes["cube"], shaders["Tema2"], modelMatrix);
    }
}
void Tema2::GenerateObstacles() {
    obstacles.clear();

    int numObstacles = 20;
    float terrainRange = 50.0f;
    float minDistanceFromOtherObstacles = 5.0f;
    float minDistanceFromDrone = 20.0f;  

    dronePosition = glm::vec3(translateX, translateY, translateZ);

    for (int i = 0; i < numObstacles; ++i) {
        glm::vec3 position;
        glm::vec3 scale;
        bool validPosition = false;

        while (!validPosition) {
            position.x = glm::linearRand(-terrainRange / 2.0f, terrainRange / 2.0f);
            position.z = glm::linearRand(-terrainRange / 2.0f, terrainRange / 2.0f);
            position.y = 0.0f;

            validPosition = true;
            for (const auto& obstacle : obstacles) {
                float distance = glm::distance(glm::vec2(position.x, position.z),
                    glm::vec2(obstacle.position.x, obstacle.position.z));
                if (distance < minDistanceFromOtherObstacles) {
                    validPosition = false;
                    break;
                }
            }

            if (glm::distance(position, dronePosition) < minDistanceFromDrone) {
                validPosition = false;
            }
        }

        scale = glm::vec3(1.0f, glm::linearRand(2.0f, 4.0f), 1.0f);

        obstacles.push_back({ position, scale });

        std::string cylinderName = "cylinder_" + std::to_string(i);
        float cylinderDistance = glm::distance(glm::vec2(position.x, position.z),
            glm::vec2(0, 0));
        if (cylinderDistance > 5.0f) {
            Mesh* cylinder = GenerateCylinder(cylinderName, 36, scale.y, 1.0f);
            meshes[cylinderName] = cylinder;


            std::string cone1Name = "cone1_" + std::to_string(i);
            std::string cone2Name = "cone2_" + std::to_string(i);
            Mesh* cone1 = GenerateCone(cone1Name, 36, 2.0f, 1.5f);
            Mesh* cone2 = GenerateCone(cone2Name, 36, 1.5f, 1.0f);
            meshes[cone1Name] = cone1;
            meshes[cone2Name] = cone2;

    
            obstacles[i].coneBaseRadius = 2.0f; 
            obstacles[i].coneHeight = scale.y;  
        }
    }
}
void Tema2::RenderObstacles() {
    for (int i = 0; i < obstacles.size(); ++i) {
        const Obstacle& obs = obstacles[i];


        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, obs.position);
        modelMatrix = glm::scale(modelMatrix, obs.scale);
        std::string cylinderName = "cylinder_" + std::to_string(i);
        RenderMesh(meshes[cylinderName], shaders["VertexNormal"], modelMatrix);

        glm::mat4 coneModelMatrix = glm::mat4(1);
        coneModelMatrix = glm::translate(coneModelMatrix, obs.position);
        float cylinderHeight = obs.scale.y;
        float cone1Height = 2.0f;
        float offset1 = 0.40f * pow(cylinderHeight, 2.05f);
        coneModelMatrix = glm::translate(coneModelMatrix, glm::vec3(0.0f, offset1, 0.0f));
        coneModelMatrix = glm::scale(coneModelMatrix, glm::vec3(1.0f, 2.0f, 1.0f));

        std::string coneName = "cone1_" + std::to_string(i);
        RenderMesh(meshes[coneName], shaders["VertexNormal"], coneModelMatrix);
        glm::mat4 cone2ModelMatrix = glm::mat4(1);
        cone2ModelMatrix = glm::translate(cone2ModelMatrix, obs.position);

        float cone2Height = 1.0f;
        float offset2 = offset1 + 3.0f; 
        cone2ModelMatrix = glm::translate(cone2ModelMatrix, glm::vec3(0.0f, offset2, 0.0f));
        cone2ModelMatrix = glm::scale(cone2ModelMatrix, glm::vec3(0.8f, 1.5f, 0.8f));

        std::string cone2Name = "cone2_" + std::to_string(i);
        RenderMesh(meshes[cone2Name], shaders["VertexNormal"], cone2ModelMatrix);
    }
}
glm::vec3 Tema2::GenerateRandomPosition(float range, float minDistance) {
    glm::vec3 position;
    bool validPosition = false;

    while (!validPosition) {
        position.x = glm::linearRand(-range / 2.0f, range / 2.0f);
        position.z = glm::linearRand(-range / 2.0f, range / 2.0f);
        position.y = 0.0f; 

        validPosition = true;

        for (const auto& obstacle : obstacles) {
            float distance = glm::distance(glm::vec2(position.x, position.z),
                glm::vec2(obstacle.position.x, obstacle.position.z));
            float distanceFromDrone = glm::distance(glm::vec2(position.x, position.z),
                glm::vec2(0, 0));
            if (distance < minDistance || distanceFromDrone < minDistance) {
                validPosition = false;
                break;
            }
        }
        for (const auto& building : buildings) {
            if (glm::distance(position, building.position) < minDistance) {
                validPosition = false;
                break;
            }
        }
    }
    return position;
}
void Tema2::GeneratePackageAndDestination() {
    float terrainRange = 75.0f;
    float minDistance = 5.0f;

    packagePosition = GenerateRandomPosition(terrainRange, minDistance);

    destinationPosition = GenerateRandomPosition(terrainRange, minDistance);
}
void Tema2::RenderPackageAndDestination() {
    if (!packagePicked) {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, packagePosition);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f));
        RenderMesh(meshes["cube"], shaders["Tema2"], modelMatrix);
    }
    else {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, packagePosition);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f));
        RenderMesh(meshes["cube"], shaders["Tema2"], modelMatrix);
    }

    if (packagePicked) {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, destinationPosition);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 0.1f, 2.0f));  // Circle size
        RenderMesh(meshes["sphere"], shaders["Tema2"], modelMatrix);
    }
}


bool Tema2::CheckCollision(const glm::vec3& objectPosition, float objectSize, const glm::vec3& targetPosition, float targetSize) {
    float distance = glm::distance(objectPosition, targetPosition);
    return distance < (objectSize + targetSize);
}
void Tema2::GenerateTerrain() {
    vector<VertexFormat> vertices;
    vector<unsigned int> indices;

    int gridWidth = 100;
    int gridHeight = 100;

    float xOffset = gridWidth / 2.0f;
    float zOffset = gridHeight / 2.0f;
    for (int i = 0; i <= gridHeight; ++i) {
        for (int j = 0; j <= gridWidth; ++j) {
            float x = j - xOffset;
            float z = i - zOffset;
            vertices.push_back(VertexFormat(glm::vec3(x, 0, z), glm::vec3(0, 1, 0), glm::vec3(0.5f, 0.8f, 0.3f)));
        }
    }

    for (int i = 0; i < gridHeight; ++i) {
        for (int j = 0; j < gridWidth; ++j) {
            int topLeft = i * (gridWidth + 1) + j;
            int topRight = topLeft + 1;
            int bottomLeft = topLeft + gridWidth + 1;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    Mesh* terrain = new Mesh("terrain");
    terrain->InitFromData(vertices, indices);
    meshes[terrain->GetMeshID()] = terrain;
}
void Tema2::RenderTerrain() {
    glm::mat4 modelMatrix = glm::mat4(1);
    RenderMesh(meshes["terrain"], shaders["VertexNormal"], modelMatrix);
}

void Tema2::Render(float deltaTimeSeconds) {
    RenderTerrain();
    RenderDrone();
    RenderObstacles();
    RenderBuildings();
    RenderPackageAndDestination();
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(45.0f), glm::vec3(0, 1, 0));
        bladeRotation += bladeRotationSpeed * deltaTimeSeconds;
    }
}
void Tema2::Update(float deltaTimeSeconds)
{
    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
    glClear(GL_DEPTH_BUFFER_BIT);  // Clear depth buffer
    projectionMatrix = glm::perspective(RADIANS(projectionFov), window->props.aspectRatio, 0.01f, 200.0f);

    glm::vec3 cameraOffset(-3.5f, 3.5f, 0.0f);
    glm::vec3 dronePosition = glm::vec3(translateX, translateY, translateZ);

    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), -angularStepOY, glm::vec3(0, 1, 0));
    glm::vec3 rotatedOffset = glm::vec3(rotationMatrix * glm::vec4(cameraOffset, 1.0f));
    glm::vec3 cameraPosition = dronePosition + rotatedOffset;

    camera->Set(cameraPosition, dronePosition, glm::vec3(0, 1, 0));
    if (!packagePicked && CheckCollision(dronePosition, 1.0f, packagePosition, 1.0f)) {
        packagePicked = true;
    }

    if (packagePicked && CheckCollision(dronePosition, 1.0f, destinationPosition, 2.0f)) {
        packagePicked = false; 
        score++;
        cout << "Score: " << score << endl;
        GeneratePackageAndDestination(); 
    }

    if (packagePicked) {
        packagePosition = dronePosition - glm::vec3(0, 1.0f, 0);
    }
    Render(deltaTimeSeconds);
    RenderPackageAndDestination();
    for (int i = 0; i < 10; i++) {
        glm::vec3 cloudPosition = glm::vec3(glm::linearRand(20.0f, 60.0f), 10.0f, glm::linearRand(20.0f, 60.0f));
        glm::mat4 cloudModelMatrix = glm::translate(glm::mat4(1.0f), cloudPosition);
        RenderMesh(meshes["cloud" + std::to_string(i)], shaders["Red"], cloudModelMatrix);
    }

    glViewport(miniViewportArea.x, miniViewportArea.y, miniViewportArea.width, miniViewportArea.height);
    glClear(GL_DEPTH_BUFFER_BIT);
    projectionMatrix = glm::ortho(-projectionWidth * 3, projectionWidth * 3, -projectionHeight * 3, projectionHeight * 3, 0.01f, 200.0f);

    glm::mat4 minimapViewMatrix = glm::lookAt(
        glm::vec3(0, 1000000, 0),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 0, -1)
    );

    RenderArrow();

    if (packagePicked) {
        RenderDestinationOnMinimap();
    }
}

void Tema2::RenderDestinationOnMinimap() {
    glm::mat4 modelMatrix = glm::mat4(1);

    modelMatrix = glm::translate(modelMatrix, destinationPosition);

    modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 0.1f, 2.0f));

    RenderMesh(meshes["sphere"], shaders["Simple"], modelMatrix);
}




void Tema2::FrameEnd()
{
}


void Tema2::RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix)

{

    if (!mesh || !shader || !shader->program)

        return;


    shader->Use();

    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));

    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));



    mesh->Render();

}



void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    float cameraSpeed = 1.0f;
    glm::vec3 newPosition = glm::vec3(translateX, translateY, translateZ);
    glm::vec3 forwardDir = glm::vec3(sin(angularStepOY), 0, -cos(angularStepOY));
    glm::vec3 rightDir = glm::vec3(cos(angularStepOY), 0, sin(angularStepOY));

    if (window->KeyHold(GLFW_KEY_A)) {
        newPosition += forwardDir * deltaTime * 2.0f;
        if (!CheckCollision(newPosition)) {
            translateX += forwardDir.x * deltaTime * cameraSpeed;
            translateZ += forwardDir.z * deltaTime * cameraSpeed;
        }
    }

    if (window->KeyHold(GLFW_KEY_S)) {
        newPosition -= rightDir * deltaTime * 2.0f;
        if (!CheckCollision(newPosition)) {
            camera->TranslateRight(-deltaTime * cameraSpeed);
            translateX -= rightDir.x * deltaTime * cameraSpeed;
            translateZ -= rightDir.z * deltaTime * cameraSpeed;
        }
    }

    if (window->KeyHold(GLFW_KEY_D)) {
        newPosition -= forwardDir * deltaTime * 2.0f;
        if (!CheckCollision(newPosition)) {
            camera->MoveForward(-deltaTime * cameraSpeed);
            translateX -= forwardDir.x * deltaTime * cameraSpeed;
            translateZ -= forwardDir.z * deltaTime * cameraSpeed;
        }

    }

    if (window->KeyHold(GLFW_KEY_W)) {
        newPosition += rightDir * deltaTime * 2.0f;
        if (!CheckCollision(newPosition)) {
            camera->TranslateRight(deltaTime * cameraSpeed);
            translateX += rightDir.x * deltaTime * cameraSpeed;
            translateZ += rightDir.z * deltaTime * cameraSpeed;
        }
    }

    if (window->KeyHold(GLFW_KEY_Q)) {
        newPosition.y -= deltaTime * 2.0f;
        if (!CheckCollision(newPosition)) {
            camera->MoveUpward(-deltaTime * cameraSpeed);
            translateY -= deltaTime * cameraSpeed;
        }
    }

    if (window->KeyHold(GLFW_KEY_E)) {
        newPosition.y += deltaTime * 2.0f;
        if (!CheckCollision(newPosition)) {
            camera->MoveUpward(deltaTime * cameraSpeed);
            translateY += deltaTime * cameraSpeed;
        }
    }

    if (window->KeyHold(GLFW_KEY_R)) {
        angularStepOY -= deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_F)) {
        angularStepOY += deltaTime;
    }


    if (window->KeyHold(GLFW_KEY_1)) {
        projectionFov -= deltaTime * 10;
    }
    if (window->KeyHold(GLFW_KEY_2)) {
        projectionFov += deltaTime * 10;
    }
    if (!CheckCollision(newPosition)) {
        translateX = newPosition.x;
        translateY = newPosition.y;
        translateZ = newPosition.z;
    }
    // TODO(student): Change projection parameters. Declare any extra
    // variables you might need in the class header. Inspect this file
    // for any hardcoded projection arguments (can you find any?) and
    // replace them with those extra variables.

}


void Tema2::OnKeyPress(int key, int mods)
{
    // Add key press event
    if (key == GLFW_KEY_T)
    {
        renderCameraTarget = !renderCameraTarget;
    }
    // TODO(student): Switch projections
    if (key == GLFW_KEY_O) orthoProjection = true;
    else if (key == GLFW_KEY_P) orthoProjection = false;
}


void Tema2::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event

    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        float sensivityOX = 0.001f;
        float sensivityOY = 0.001f;

        if (window->GetSpecialKeyState() == 0) {
            renderCameraTarget = false;
            // TODO(student): Rotate the camera in first-person mode around
            // OX and OY using `deltaX` and `deltaY`. Use the sensitivity
            // variables for setting up the rotation speed.
            camera->RotateFirstPerson_OX(-deltaY * sensivityOX);
            camera->RotateFirstPerson_OY(-deltaX * sensivityOY);
        }

        if (window->GetSpecialKeyState() & GLFW_MOD_CONTROL) {
            renderCameraTarget = true;
            // TODO(student): Rotate the camera in third-person mode around
            // OX and OY using `deltaX` and `deltaY`. Use the sensitivity
            // variables for setting up the rotation speed.
            camera->RotateThirdPerson_OX(-deltaY * sensivityOX);
            camera->RotateThirdPerson_OY(-deltaX * sensivityOY);
        }
    }
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema2::OnWindowResize(int width, int height)
{
}