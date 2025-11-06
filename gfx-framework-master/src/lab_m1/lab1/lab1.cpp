#include "lab_m1/lab1/lab1.h"

#include <iostream>

using namespace std;
using namespace m1;

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */

Lab1::Lab1()
{
    // TODO(student): Never forget to initialize class variables!
    clearColorR = 0.0f;
    clearColorG = 0.0f;
    clearColorB = 0.0f;

    objectPosition = glm::vec3(0.0f, 0.5f, 0.0f);
    objectScale = glm::vec3(1.0f);

    currentMeshIndex = 0;

    moveSpeed = 2.0f;
}


Lab1::~Lab1()
{
}


void Lab1::Init()
{
    // Load a mesh from file into GPU memory. We only need to do it once,
    // no matter how many times we want to draw this mesh.
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
        Mesh* mesh = new Mesh("teapot");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "teapot.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    meshCycle = { "box", "sphere", "teapot" };
}


void Lab1::FrameStart()
{
}


void Lab1::Update(float deltaTimeSeconds)
{
    glm::ivec2 resolution = window->props.resolution;

    // Sets the clear color for the color buffer
    glClearColor(clearColorR, clearColorG, clearColorB, 1);

    // Clears the color buffer (using the previously set color) and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);

    // Render mesh currentMesh at position objectPosition with scale objectScale
    std::string currentMesh = meshCycle[currentMeshIndex];
    RenderMesh(meshes[currentMesh], objectPosition, objectScale);

    // RenderMesh(meshes["box"], objectPosition + glm::vec3(1, 0, 0), glm::vec3(0.5f)); // example additional render
}


void Lab1::FrameEnd()
{
    DrawCoordinateSystem();
}


void Lab1::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input

    // Move mesh instance with WASD keys + QE for vertical movement
    if (window->KeyHold(GLFW_KEY_W)) {
        objectPosition.z -= moveSpeed * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_S)) {
        objectPosition.z += moveSpeed * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_A)) {
        objectPosition.x -= moveSpeed * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_D)) {
        objectPosition.x += moveSpeed * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_Q)) {
        objectPosition.y -= moveSpeed * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_E)) {
        objectPosition.y += moveSpeed * deltaTime;
    }
}


void Lab1::OnKeyPress(int key, int mods)
{
    // Add key press event
    if (key == GLFW_KEY_F) {
        // TODO(student): Change the values of the color components.
        clearColorR = static_cast<float>(rand()) / RAND_MAX;
        clearColorG = static_cast<float>(rand()) / RAND_MAX;
        clearColorB = static_cast<float>(rand()) / RAND_MAX;
    }

    // Cycle through meshes with M key
    if (key == GLFW_KEY_M) {
        currentMeshIndex = (currentMeshIndex + 1) % meshCycle.size();
    }

    // Scale down with left bracket, scale up with right bracket
    if (key == GLFW_KEY_LEFT_BRACKET) {
        objectScale *= 0.9f;
    }
    if (key == GLFW_KEY_RIGHT_BRACKET) {
        objectScale *= 1.1f;
    }
}


void Lab1::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Lab1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Lab1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Lab1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Lab1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Lab1::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
