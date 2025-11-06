#include "lab_m1/lab2/lab2.h"

#include <vector>
#include <iostream>
#include <cmath>

#include "core/engine.h"
#include "utils/gl_utils.h"

using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Lab2::Lab2()
{
}


Lab2::~Lab2()
{
}


void Lab2::Init()
{
    cullFace = GL_BACK;
    polygonMode = GL_FILL;

    {
        Mesh* mesh = new Mesh("teapot");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "teapot.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        vector<VertexFormat> vertices
        {
            VertexFormat(glm::vec3(-1, -1,  1), glm::vec3(0, 0, 0)), // 0
            VertexFormat(glm::vec3( 1, -1,  1), glm::vec3(0, 1, 0)), // 1
            VertexFormat(glm::vec3(-1,  1,  1), glm::vec3(0, 0, 1)), // 2
            VertexFormat(glm::vec3( 1,  1,  1), glm::vec3(1, 1, 0)), // 3

            VertexFormat(glm::vec3(-1, -1, -1), glm::vec3(1, 0, 1)), // 4
            VertexFormat(glm::vec3( 1, -1, -1), glm::vec3(0, 1, 1)), // 5
            VertexFormat(glm::vec3(-1,  1, -1), glm::vec3(1, 1, 1)), // 6
            VertexFormat(glm::vec3( 1,  1, -1), glm::vec3(0.2f, 0.8f, 0.6f))  // 7
        };

        vector<unsigned int> indices =
        {
            // Front face
            0, 1, 2,
            1, 3, 2,

            // Right face
            1, 5, 3,
            5, 7, 3,

            // Back face
            5, 4, 7,
            4, 6, 7,

            // Left face
            4, 0, 6,
            0, 2, 6,

            // Bottom face
            4, 5, 0,
            5, 1, 0,

            // Top face
            2, 3, 6,
            3, 7, 6
        };

        // meshes["cube_A"] = new Mesh("generated cube 1");
        // meshes["cube_A"]->InitFromData(vertices, indices);

        Mesh* mesh = new Mesh("bunny");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "animals"), "bunny.obj");
        meshes[mesh->GetMeshID()] = mesh;

        CreateMesh("cube", vertices, indices);
    }

    // Create a tetrahedron mesh with per-face colors (blue, yellow, red, dark gray)
    {
        // Define the 4 geometric points
        glm::vec3 A( 0.f,  1.f,  0.f);
        glm::vec3 B(-1.f, -1.f,  1.f);
        glm::vec3 C( 1.f, -1.f,  1.f);
        glm::vec3 D( 0.f, -1.f, -1.f);

        // Colors
        glm::vec3 BLUE(0.f, 0.f, 1.f);
        glm::vec3 YELLOW(1.f, 1.f, 0.f);
        glm::vec3 RED(1.f, 0.f, 0.f);
        glm::vec3 DARKGRAY(0.3f, 0.3f, 0.3f);

        // 12 vertices (3 per face) so each face is a solid color
        vector<VertexFormat> tv = {
            // Face 1 (A, B, C) - Blue
            VertexFormat(A, BLUE), VertexFormat(B, BLUE), VertexFormat(C, BLUE),
            // Face 2 (A, C, D) - Yellow
            VertexFormat(A, YELLOW), VertexFormat(C, YELLOW), VertexFormat(D, YELLOW),
            // Face 3 (A, D, B) - Red
            VertexFormat(A, RED), VertexFormat(D, RED), VertexFormat(B, RED),
            // Face 4 (B, D, C) - Dark gray (base)
            VertexFormat(B, DARKGRAY), VertexFormat(D, DARKGRAY), VertexFormat(C, DARKGRAY)
        };
        vector<unsigned int> ti = {
            0, 1, 2,
            3, 4, 5,
            6, 7, 8,
            9, 10, 11
        };
        CreateMesh("tetra", tv, ti);
    }

    // Create a circle mesh (XZ plane at y ~ 0)
    {
        const int segments = 64;
        const float radius = 1.0f;
        const float y = 0.01f;
        const float TAU = 6.28318530718f;

        vector<VertexFormat> cv;
        vector<unsigned int> ci;

        // center vertex (magenta)
        cv.emplace_back(glm::vec3(0.f, y, 0.f), glm::vec3(1.f, 0.f, 1.f));

        // ring vertices CCW seen from +Y
        for (int i = 0; i <= segments; ++i)
        {
            float t = (float)i / segments;
            float ang = t * TAU;
            float x = radius * cosf(ang);
            float z = radius * sinf(ang);
            cv.emplace_back(glm::vec3(x, y, z), glm::vec3(0.2f, 0.7f, 1.0f));
        }
        
        // triangles fan from center
        for (int i = 1; i <= segments; ++i)
        {
            ci.push_back(0);
            ci.push_back(i);
            ci.push_back(i + 1);
        }

        CreateMesh("circle", cv, ci);
    }

    {
        vector<VertexFormat> sv = {
            VertexFormat(glm::vec3(-1.f, 0.f, -1.f), glm::vec3(1, 0, 1)),
            VertexFormat(glm::vec3( 1.f, 0.f, -1.f), glm::vec3(0, 1, 1)),
            VertexFormat(glm::vec3(-1.f, 0.f,  1.f), glm::vec3(1, 1, 0)),
            VertexFormat(glm::vec3( 1.f, 0.f,  1.f), glm::vec3(0.2f, 0.8f, 0.6f))
        };

        vector<unsigned int> si = {
            0, 1, 2,
            1, 2, 3
        };
        CreateMesh("square_twist", sv, si);
    }
}


void Lab2::CreateMesh(const char *name, const std::vector<VertexFormat> &vertices, const std::vector<unsigned int> &indices)
{
    unsigned int VAO = 0;
    // Create the VAO and bind it
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO = 0;
    // Create the VBO and bind it
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // Send vertices data into the VBO buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    unsigned int IBO = 0;
    // Create the IBO and bind it
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    // Send indices data into the IBO buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    // ========================================================================
    // This section demonstrates how the GPU vertex shader program
    // receives data. It will be learned later, when GLSL shaders will be
    // introduced. For the moment, just think that each property value from
    // our vertex format needs to be sent to a certain channel, in order to
    // know how to receive it in the GLSL vertex shader.

    // Set vertex position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);

    // Set vertex normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));

    // Set texture coordinate attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3)));

    // Set vertex color attribute
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));
    // ========================================================================
    
    // Unbind the VAO
    glBindVertexArray(0);

    // Check for OpenGL errors
    if (GetOpenGLError() == GL_INVALID_OPERATION)
    {
        cout << "\t[NOTE] : For students : DON'T PANIC! This error should go away when completing the tasks." << std::endl;
        cout << "\t[NOTE] : For developers : This happens because OpenGL core spec >=3.1 forbids null VAOs." << std::endl;
    }

    // Mesh information is saved into a Mesh object
    meshes[name] = new Mesh(name);
    meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
}


void Lab2::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();

    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}


void Lab2::Update(float deltaTimeSeconds)
{
    glLineWidth(3);
    glPointSize(5);
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

    // Enable face culling
    glEnable(GL_CULL_FACE);

    // Set face custom culling. Use the `cullFace` variable.
    glCullFace(cullFace);

    // Render an object using face normals for color
    RenderMesh(meshes["teapot"], shaders["VertexNormal"], glm::vec3(0, 0.5f, -1.5f), glm::vec3(0.75f));

    // Render an object using colors from vertex
    RenderMesh(meshes["bunny"], shaders["VertexNormal"], glm::vec3(-1.5, 1, 1), glm::vec3(0.05f));

    // Draw the mesh that was created with `CreateMesh()`
    RenderMesh(meshes["cube"], shaders["VertexColor"], glm::vec3(3.5f, 0.5f, 0), glm::vec3(0.25f));

    // Draw the tetrahedron
    RenderMesh(meshes["tetra"], shaders["VertexColor"], glm::vec3(0.f, 0.5f, 1.5f), glm::vec3(0.6f));

    // Draw the square (two triangles with opposing orientation)
    RenderMesh(meshes["square_twist"], shaders["VertexColor"], glm::vec3(0.f, 0.01f, 0.f), glm::vec3(0.75f));

    // Draw the circle
    RenderMesh(meshes["circle"], shaders["VertexColor"], glm::vec3(-3.0f, 0.0f, 0.0f), glm::vec3(1.0f));

    // Disable face culling
    glDisable(GL_CULL_FACE);
}


void Lab2::FrameEnd()
{
    DrawCoordinateSystem();
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Lab2::OnInputUpdate(float deltaTime, int mods)
{
}


void Lab2::OnKeyPress(int key, int mods)
{
    // Switch between GL_FRONT and GL_BACK culling.
    // Save the state in `cullFace` variable and apply it in the
    // `Update()` method, NOT here!

    if (key == GLFW_KEY_F2)
    {
        cullFace = (cullFace == GL_BACK) ? GL_FRONT : GL_BACK;
    }

    if (key == GLFW_KEY_SPACE)
    {
        switch (polygonMode)
        {
        case GL_POINT:
            polygonMode = GL_FILL;
            break;
        case GL_LINE:
            polygonMode = GL_POINT;
            break;
        default:
            polygonMode = GL_LINE;
            break;
        }
    }
}


void Lab2::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Lab2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Lab2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Lab2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Lab2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Lab2::OnWindowResize(int width, int height)
{
}
