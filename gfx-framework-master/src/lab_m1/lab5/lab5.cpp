#include "lab_m1/lab5/lab5.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Lab5::Lab5()
{
}


Lab5::~Lab5()
{
}


void Lab5::Init()
{
    renderCameraTarget = false;

    camera = new implemented::Camera();
    camera->Set(glm::vec3(0, 2, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

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

    // Setup initial projection parameters using class variables
    isPerspective = true;
    fovY = RADIANS(60.0f);
    zNear = 0.01f;
    zFar = 200.0f;
    orthoHeight = 5.0f;
    // Keep width consistent with aspect to avoid stretching in ortho
    orthoWidth = orthoHeight * window->props.aspectRatio;

    projectionMatrix = glm::perspective(fovY, window->props.aspectRatio, zNear, zFar);
}


void Lab5::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}


void Lab5::Update(float deltaTimeSeconds)
{
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(45.0f), glm::vec3(0, 1, 0));

        RenderMesh(meshes["box"], shaders["VertexNormal"], modelMatrix);
    }

    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(2, 0.5f, 0));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(60.0f), glm::vec3(1, 0, 0));
        RenderMesh(meshes["box"], shaders["VertexNormal"], modelMatrix);
    }

    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-2, 0.5f, 0));
        RenderMesh(meshes["box"], shaders["Simple"], modelMatrix);
    }

    // Extra objects with different transforms
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.5f, 1.0f, -2.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 1.5f, 0.5f));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(25.0f), glm::vec3(0, 1, 0));
        RenderMesh(meshes["box"], shaders["VertexNormal"], modelMatrix);
    }

    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(1.5f, 0.5f, 2.0f));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(90.0f), glm::vec3(0, 0, 1));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.75f));
        RenderMesh(meshes["sphere"], shaders["VertexNormal"], modelMatrix);
    }

    // Render the camera target. This is useful for understanding where
    // the rotation point is, when moving in third-person camera mode.
    if (renderCameraTarget)
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, camera->GetTargetPosition());
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
        RenderMesh(meshes["sphere"], shaders["VertexNormal"], modelMatrix);
    }
}


void Lab5::FrameEnd()
{
    DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);
}


void Lab5::RenderMesh(Mesh * mesh, Shader * shader, const glm::mat4 & modelMatrix)
{
    if (!mesh || !shader || !shader->program)
        return;

    // Render an object using the specified shader and the specified position
    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    mesh->Render();
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Lab5::OnInputUpdate(float deltaTime, int mods)
{
    // move the camera only if MOUSE_RIGHT button is pressed
    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        float cameraSpeed = 2.0f;

        if (window->KeyHold(GLFW_KEY_W)) {
            camera->MoveForward(cameraSpeed * deltaTime);
        }

        if (window->KeyHold(GLFW_KEY_A)) {
            camera->TranslateRight(-cameraSpeed * deltaTime);
        }

        if (window->KeyHold(GLFW_KEY_S)) {
            camera->MoveForward(-cameraSpeed * deltaTime);
        }

        if (window->KeyHold(GLFW_KEY_D)) {
            camera->TranslateRight(cameraSpeed * deltaTime);
        }

        if (window->KeyHold(GLFW_KEY_Q)) {
            camera->TranslateUpward(-cameraSpeed * deltaTime);
        }

        if (window->KeyHold(GLFW_KEY_E)) {
            camera->TranslateUpward(cameraSpeed * deltaTime);
        }
    }

    // Projection parameter adjustments
    const float fovSpeed = RADIANS(30.0f) * deltaTime; // 30 deg per second
    const float orthoSpeed = 2.0f * deltaTime;

    if (isPerspective) {
        // Increase FoV
        if (window->KeyHold(GLFW_KEY_KP_ADD) || window->KeyHold(GLFW_KEY_EQUAL)) {
            fovY += fovSpeed;
            fovY = glm::clamp(fovY, RADIANS(10.0f), RADIANS(170.0f));
            projectionMatrix = glm::perspective(fovY, window->props.aspectRatio, zNear, zFar);
        }
        // Decrease FoV
        if (window->KeyHold(GLFW_KEY_KP_SUBTRACT) || window->KeyHold(GLFW_KEY_MINUS)) {
            fovY -= fovSpeed;
            fovY = glm::clamp(fovY, RADIANS(10.0f), RADIANS(170.0f));
            projectionMatrix = glm::perspective(fovY, window->props.aspectRatio, zNear, zFar);
        }
    } else {
        // Adjust ortho height and keep width consistent with aspect
        if (window->KeyHold(GLFW_KEY_KP_ADD) || window->KeyHold(GLFW_KEY_EQUAL)) {
            orthoHeight += orthoSpeed;
            orthoHeight = glm::max(0.1f, orthoHeight);
            orthoWidth = orthoHeight * window->props.aspectRatio;
            projectionMatrix = glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, zNear, zFar);
        }
        if (window->KeyHold(GLFW_KEY_KP_SUBTRACT) || window->KeyHold(GLFW_KEY_MINUS)) {
            orthoHeight = glm::max(0.1f, orthoHeight - orthoSpeed);
            orthoWidth = orthoHeight * window->props.aspectRatio;
            projectionMatrix = glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, zNear, zFar);
        }
    }
}


void Lab5::OnKeyPress(int key, int mods)
{
    // Add key press event
    if (key == GLFW_KEY_T)
    {
        renderCameraTarget = !renderCameraTarget;
    }
    // Switch projections
    if (key == GLFW_KEY_P) {
        isPerspective = true;
        projectionMatrix = glm::perspective(fovY, window->props.aspectRatio, zNear, zFar);
    }
    if (key == GLFW_KEY_O) {
        isPerspective = false;
        // keep consistent aspect to avoid stretching
        orthoWidth = orthoHeight * window->props.aspectRatio;
        projectionMatrix = glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, -2.0f, zFar);
    }
}


void Lab5::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Lab5::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event

    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        float sensivityOX = 0.001f;
        float sensivityOY = 0.001f;

        if (window->GetSpecialKeyState() == 0) {
            renderCameraTarget = false;
            camera->RotateFirstPerson_OX(-sensivityOY * deltaY);
            camera->RotateFirstPerson_OY(-sensivityOX * deltaX);
        }

        if (window->GetSpecialKeyState() & GLFW_MOD_CONTROL) {
            renderCameraTarget = true;
            camera->RotateThirdPerson_OX(-sensivityOY * deltaY);
            camera->RotateThirdPerson_OY(-sensivityOX * deltaX);
        }
    }
}


void Lab5::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Lab5::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Lab5::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Lab5::OnWindowResize(int width, int height)
{
    // Update projection on resize to keep aspect consistent
    float aspect = (height != 0) ? (static_cast<float>(width) / static_cast<float>(height)) : window->props.aspectRatio;
    if (isPerspective) {
        projectionMatrix = glm::perspective(fovY, aspect, zNear, zFar);
    } else {
        orthoWidth = orthoHeight * aspect;
        projectionMatrix = glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, zNear, zFar);
    }
}
