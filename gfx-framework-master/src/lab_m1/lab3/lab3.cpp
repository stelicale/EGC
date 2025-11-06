#include "lab_m1/lab3/lab3.h"

#include <vector>
#include <iostream>
#include <cmath>

#include "lab_m1/lab3/transform2D.h"
#include "lab_m1/lab3/object2D.h"

using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Lab3::Lab3()
{
}


Lab3::~Lab3()
{
}


void Lab3::Init()
{
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    glm::vec3 corner = glm::vec3(0, 0, 0);
    float squareSide = 100;

    // TODO(student): Compute coordinates of a square's center, and store
    // then in the `cx` and `cy` class variables (see the header). Use
    // `corner` and `squareSide`. These two class variables will be used
    // in the `Update()` function. Think about it, why do you need them?

    cx = corner.x + squareSide / 2;
    cy = corner.y + squareSide / 2;

    // Initialize tx and ty (the translation steps)
    translateX = 0;
    translateY = 0;

    // Initialize sx and sy (the scale factors)
    scaleX = 1;
    scaleY = 1;

    // Initialize angularStep
    angularStep = 0;

    Mesh* square1 = object2D::CreateSquare("square1", corner, squareSide, glm::vec3(1, 0, 0), true);
    AddMeshToList(square1);

    Mesh* square2 = object2D::CreateSquare("square2", corner, squareSide, glm::vec3(0, 1, 0), true);
    AddMeshToList(square2);

    Mesh* square3 = object2D::CreateSquare("square3", corner, squareSide, glm::vec3(0, 0, 1), true);
    AddMeshToList(square3);

    Mesh* square4 = object2D::CreateSquare("square4", corner, squareSide, glm::vec3(1, 1, 0), true);
    AddMeshToList(square4);
}


void Lab3::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}


void Lab3::Update(float deltaTimeSeconds)
{
    // TODO(student): Update steps for translation, rotation and scale,
    // in order to create animations. Use the class variables in the
    // class header, and if you need more of them to complete the task,
    // add them over there!
    float sDelta = scaleSpeed * deltaTimeSeconds * (float)scaleDir;
    scaleX += sDelta;
    scaleY += sDelta;

    if (scaleX >= scaleMax) {
        scaleX = scaleMax;
        scaleY = scaleMax;
        scaleDir = -1;
    }
    else if (scaleX <= scaleMin) {
        scaleX = scaleMin;
        scaleY = scaleMin;
        scaleDir = 1;
    }

    // Square 1: scale around its center at (cx, cy)
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(150, 250);
    modelMatrix *= transform2D::Translate(cx, cy);
    modelMatrix *= transform2D::Scale(scaleX, scaleY);
    modelMatrix *= transform2D::Translate(-cx, -cy);
    RenderMesh2D(meshes["square1"], shaders["VertexColor"], modelMatrix);

    // Square 2: rotation around center
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(400, 250);
    modelMatrix *= transform2D::Translate(cx, cy);
    angularStep += glm::radians(90.0f) * deltaTimeSeconds; // rotate 90 degrees per second
    modelMatrix *= transform2D::Rotate(angularStep);
    modelMatrix *= transform2D::Translate(-cx, -cy);
    RenderMesh2D(meshes["square2"], shaders["VertexColor"], modelMatrix);

    // Square 3: independent vertical ping-pong translation
    float tDelta = translateSpeedY * deltaTimeSeconds * (float)moveDirY;
    translateY += tDelta;
    if (translateY >= translateMax) { translateY = translateMax; moveDirY = -1; }
    if (translateY <= translateMin) { translateY = translateMin; moveDirY = 1; }
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(650, 250);
    modelMatrix *= transform2D::Translate(cx, cy);
    modelMatrix *= transform2D::Translate(0, translateY);
    modelMatrix *= transform2D::Translate(-cx, -cy);
    RenderMesh2D(meshes["square3"], shaders["VertexColor"], modelMatrix);

    // Pentagon from five squares (center empty)
    {
        glm::vec2 center(750.f, 200.f);
        float R = 150;          // reduced radius to bring squares closer
        float s = 1.0f;           // scale of each square
        float rot = 0; // rotate every square
        for (int i = 0; i < 5; ++i)
        {
            float angDeg = -90.f + i * 72.f;
            float ang = glm::radians(angDeg);
            float px = center.x + R * cosf(ang);
            float py = center.y + R * sinf(ang);
            if (i == 1)
                rot += glm::radians(45.0f);
            modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(center.x, center.y);
            modelMatrix *= transform2D::Translate(cx, cy);
            modelMatrix *= transform2D::Rotate(rot);
            modelMatrix *= transform2D::Scale(s, s);
            modelMatrix *= transform2D::Translate(-cx, -cy);
            RenderMesh2D(meshes["square4"], shaders["VertexColor"], modelMatrix);
        }
    }
}


void Lab3::FrameEnd()
{
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Lab3::OnInputUpdate(float deltaTime, int mods)
{
    // Add key press event
}


void Lab3::OnKeyPress(int key, int mods)
{
    // Add key press event
}


void Lab3::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Lab3::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Lab3::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Lab3::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Lab3::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Lab3::OnWindowResize(int width, int height)
{
}
