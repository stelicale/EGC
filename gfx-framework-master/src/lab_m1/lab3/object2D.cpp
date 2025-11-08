#include "object2D.h"

#include <vector>
#include <cmath>

#include "core/engine.h"
#include "utils/gl_utils.h"

// Use a float PI to avoid implicit double->float conversions (C4244)
static constexpr float PI_F = 3.14159265358979323846f;

Mesh* object2D::CreateSquare(
    const std::string& name,
    glm::vec3 leftBottomCorner,
    float length,
    glm::vec3 color,
    bool fill)
{
    glm::vec3 corner = leftBottomCorner;

    std::vector<VertexFormat> vertices =
    {
        VertexFormat(corner, color),
        VertexFormat(corner + glm::vec3(length, 0, 0), color),
        VertexFormat(corner + glm::vec3(length, length, 0), color),
        VertexFormat(corner + glm::vec3(0, length, 0), color)
    };

    Mesh* square = new Mesh(name);
    std::vector<unsigned int> indices = { 0, 1, 2, 3 };

    if (!fill) {
        square->SetDrawMode(GL_LINE_LOOP);
    }
    else {
        // Draw 2 triangles. Add the remaining 2 indices
        indices.push_back(0);
        indices.push_back(2);
    }

    square->InitFromData(vertices, indices);
    return square;
}

Mesh* object2D::CreateRectangle(
    const std::string& name,
    glm::vec3 leftBottomCorner,
    float width,
    float height,
    glm::vec3 color,
    bool fill)
{
    glm::vec3 corner = leftBottomCorner;

    std::vector<VertexFormat> vertices =
    {
        VertexFormat(corner, color),
        VertexFormat(corner + glm::vec3(width, 0, 0), color),
        VertexFormat(corner + glm::vec3(width, height, 0), color),
        VertexFormat(corner + glm::vec3(0, height, 0), color)
    };

    Mesh* rectangle = new Mesh(name);
    std::vector<unsigned int> indices = { 0, 1, 2, 3 };

    if (!fill) {
        rectangle->SetDrawMode(GL_LINE_LOOP);
    }
    else {
        // Dreptunghiul e compus din 2 triunghiuri
        indices.push_back(0);
        indices.push_back(2);
    }

    rectangle->InitFromData(vertices, indices);
    return rectangle;
}

Mesh* object2D::CreateCircle(
    const std::string& name,
    glm::vec3 center,
    float radius,
    glm::vec3 color,
    bool fill,
    int numSegments)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Centrul cercului
    vertices.push_back(VertexFormat(center, color));

    // Generăm punctele pe circumferință
    for (int i = 0; i <= numSegments; i++) {
        float angle = (2.0f * PI_F * static_cast<float>(i)) / static_cast<float>(numSegments);
        float x = center.x + radius * static_cast<float>(std::cos(angle));
        float y = center.y + radius * static_cast<float>(std::sin(angle));
        vertices.push_back(VertexFormat(glm::vec3(x, y, center.z), color));
    }

    Mesh* circle = new Mesh(name);

    if (!fill) {
        // doar linia cercului
        for (int i = 0; i <= numSegments; i++)
            indices.push_back(i);
        circle->SetDrawMode(GL_LINE_LOOP);
    }
    else {
        // triunghiuri din centru spre fiecare segment
        for (int i = 0; i <= numSegments; i++) {
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }
    }

    circle->InitFromData(vertices, indices);
    return circle;
}

Mesh* object2D::CreateBumper(
    const std::string& name,
    glm::vec3 center,
    float radius,
    float squareSize,
    glm::vec3 semiColor,
    glm::vec3 squareColor,
    bool fill,
    int numSegments)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // --- Semioval ---
    // Height (drawn) = squareSize, Width = 3 * squareSize
    // Base of semioval coincides with top edge of the square (y = center.y)
    float ovalRadiusX = 1.5f * squareSize;           // half width
    float ovalRadiusY = squareSize;                  // drawn height (top half height)
    glm::vec3 ovalCenter = center;                   // base line

    int baseIdx = 0;
    vertices.push_back(VertexFormat(ovalCenter, semiColor)); // center on base line
    for (int i = 0; i <= numSegments; i++) {
        float angle = (PI_F * static_cast<float>(i)) / static_cast<float>(numSegments); // 0..pi (top half)
        float x = ovalCenter.x + ovalRadiusX * static_cast<float>(std::cos(angle));
        float y = ovalCenter.y + ovalRadiusY * static_cast<float>(std::sin(angle));
        vertices.push_back(VertexFormat(glm::vec3(x, y, ovalCenter.z), semiColor));
    }
    if (fill) {
        for (int i = 0; i < numSegments; i++) {
            indices.push_back(baseIdx);
            indices.push_back(baseIdx + i + 1);
            indices.push_back(baseIdx + i + 2);
        }
    }
    else {
        for (int i = 0; i <= numSegments; i++)
            indices.push_back(baseIdx + i + 1);
    }

    // --- Square under the semioval ---
    float squareX = center.x - squareSize / 2.0f;
    float squareY = center.y - squareSize;
    int squareStart = (int)vertices.size();
    vertices.push_back(VertexFormat(glm::vec3(squareX, squareY, center.z), squareColor)); // LB
    vertices.push_back(VertexFormat(glm::vec3(squareX + squareSize, squareY, center.z), squareColor)); // RB
    vertices.push_back(VertexFormat(glm::vec3(squareX + squareSize, squareY + squareSize, center.z), squareColor)); // RT
    vertices.push_back(VertexFormat(glm::vec3(squareX, squareY + squareSize, center.z), squareColor)); // LT

    if (fill) {
        indices.push_back(squareStart + 0);
        indices.push_back(squareStart + 1);
        indices.push_back(squareStart + 2);
        indices.push_back(squareStart + 0);
        indices.push_back(squareStart + 2);
        indices.push_back(squareStart + 3);
    }
    else {
        indices.push_back(squareStart + 0);
        indices.push_back(squareStart + 1);
        indices.push_back(squareStart + 2);
        indices.push_back(squareStart + 3);
        indices.push_back(squareStart + 0);
    }

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);
    return mesh;
}

Mesh* object2D::CreateCannon(
    const std::string& name,
    glm::vec3 baseCorner,
    float squareSize,
    glm::vec3 blackColor,
    glm::vec3 whiteColor,
    bool fill)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // 3 pătrate pe verticală: negru, alb, negru
    for (int i = 0; i < 3; i++) {
        glm::vec3 color = (i == 0) ? whiteColor : blackColor;
        int vStart = (int)vertices.size();
        float y = baseCorner.y + i * squareSize;
        vertices.push_back(VertexFormat(glm::vec3(baseCorner.x, y, baseCorner.z), color)); // stânga jos
        vertices.push_back(VertexFormat(glm::vec3(baseCorner.x + squareSize, y, baseCorner.z), color)); // dreapta jos
        vertices.push_back(VertexFormat(glm::vec3(baseCorner.x + squareSize, y + squareSize, baseCorner.z), color)); // dreapta sus
        vertices.push_back(VertexFormat(glm::vec3(baseCorner.x, y + squareSize, baseCorner.z), color)); // stânga sus

        if (fill) {
            // 2 triunghiuri
            indices.push_back(vStart + 0);
            indices.push_back(vStart + 1);
            indices.push_back(vStart + 2);

            indices.push_back(vStart + 0);
            indices.push_back(vStart + 2);
            indices.push_back(vStart + 3);
        }
        else {
            indices.push_back(vStart + 0);
            indices.push_back(vStart + 1);
            indices.push_back(vStart + 2);
            indices.push_back(vStart + 3);
            indices.push_back(vStart + 0);
        }
    }

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);
    return mesh;
}
