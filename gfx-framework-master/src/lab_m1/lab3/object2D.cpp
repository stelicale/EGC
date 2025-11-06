#include "object2D.h"

#include <vector>
#include <cmath>

#include "core/engine.h"
#include "utils/gl_utils.h"


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

Mesh* object2D::CreateTriangle(
    const std::string& name,
    glm::vec3 bottomLeftCorner,
    float base,
    float height,
    glm::vec3 color,
    bool fill)
{
    glm::vec3 corner = bottomLeftCorner;

    std::vector<VertexFormat> vertices =
    {
        VertexFormat(corner, color),                                   // stânga jos
        VertexFormat(corner + glm::vec3(base, 0, 0), color),           // dreapta jos
        VertexFormat(corner + glm::vec3(base / 2, height, 0), color)   // vârful sus
    };

    Mesh* triangle = new Mesh(name);
    std::vector<unsigned int> indices = { 0, 1, 2 };

    if (!fill) {
        triangle->SetDrawMode(GL_LINE_LOOP);
    }

    triangle->InitFromData(vertices, indices);
    return triangle;
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
        float angle = 2 * M_PI * i / numSegments;
        float x = center.x + radius * cos(angle);
        float y = center.y + radius * sin(angle);
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

Mesh* object2D::CreateSemiCircle(
    const std::string& name,
    glm::vec3 center,
    float radius,
    glm::vec3 color,
    bool fill,
    int numSegments)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Centrul semicercului
    vertices.push_back(VertexFormat(center, color));

    // Generăm punctele pe jumătate de cerc (de la 0 la PI)
    for (int i = 0; i <= numSegments; i++) {
        float angle = M_PI * i / numSegments;   // 180° = π radiani
        float x = center.x + radius * cos(angle);
        float y = center.y + radius * sin(angle);
        vertices.push_back(VertexFormat(glm::vec3(x, y, center.z), color));
    }

    Mesh* semicircle = new Mesh(name);

    if (!fill) {
        // Desenează doar conturul semicercului
        for (int i = 0; i <= numSegments; i++) {
            indices.push_back(i);
        }
        semicircle->SetDrawMode(GL_LINE_STRIP);
    }
    else {
        // Umple semicercul cu triunghiuri
        for (int i = 0; i <= numSegments; i++) {
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }
    }

    semicircle->InitFromData(vertices, indices);
    return semicircle;
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
        float angle = M_PI * i / numSegments; // 0..pi (top half)
        float x = ovalCenter.x + ovalRadiusX * cos(angle);
        float y = ovalCenter.y + ovalRadiusY * sin(angle);
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
    if (!fill)
        mesh->SetDrawMode(GL_LINE_LOOP);
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
    if (!fill)
        mesh->SetDrawMode(GL_LINE_LOOP);
    return mesh;
}

Mesh* object2D::CreateLetterS(
    const std::string& name,
    glm::vec3 origin,
    float size,
    glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float w = size * 0.6f;  // lățime
    float h = size;         // înălțime
    float t = size * 0.15f; // grosime linie

    // Bara de sus (orizontală)
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h - t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h - t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h, 0), color));

    // Bara din mijloc
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h / 2 - t / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h / 2 - t / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h / 2 + t / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h / 2 + t / 2, 0), color));

    // Bara de jos
    vertices.push_back(VertexFormat(origin + glm::vec3(0, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, t, 0), color));

    // Linie verticală stânga sus
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(t, h / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(t, h, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h, 0), color));

    // Linie verticală dreapta jos
    vertices.push_back(VertexFormat(origin + glm::vec3(w - t, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w - t, h / 2, 0), color));

    // Indici pentru 5 dreptunghiuri (fiecare cu 2 triunghiuri)
    for (int i = 0; i < 5; i++) {
        int base = i * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);
    return mesh;
}

Mesh* object2D::CreateLetterC(
    const std::string& name,
    glm::vec3 origin,
    float size,
    glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float w = size * 0.6f;
    float h = size;
    float t = size * 0.15f;

    // Bara de sus
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h - t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h - t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h, 0), color));

    // Bara de jos
    vertices.push_back(VertexFormat(origin + glm::vec3(0, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, t, 0), color));

    // Linie verticală stânga
    vertices.push_back(VertexFormat(origin + glm::vec3(0, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(t, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(t, h, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h, 0), color));

    for (int i = 0; i < 3; i++) {
        int base = i * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);
    return mesh;
}

Mesh* object2D::CreateLetterO(
    const std::string& name,
    glm::vec3 origin,
    float size,
    glm::vec3 color)
{
    // Letter 'O' stays as the rectangular-styled O (box with rounded thickness via rectangles)
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float w = size * 0.6f;
    float h = size;
    float t = size * 0.15f;

    // Bara de sus
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h - t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h - t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h, 0), color));

    // Bara de jos
    vertices.push_back(VertexFormat(origin + glm::vec3(0, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, t, 0), color));

    // Linie verticală stânga
    vertices.push_back(VertexFormat(origin + glm::vec3(0, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(t, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(t, h, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h, 0), color));

    // Linie verticală dreapta
    vertices.push_back(VertexFormat(origin + glm::vec3(w - t, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w - t, h, 0), color));

    for (int i = 0; i < 4; i++) {
        int base = i * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);
    return mesh;
}

Mesh* object2D::CreateLetterR(
    const std::string& name,
    glm::vec3 origin,
    float size,
    glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float w = size * 0.6f;
    float h = size;
    float t = size * 0.15f;

    // Linie verticală stânga
    vertices.push_back(VertexFormat(origin + glm::vec3(0, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(t, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(t, h, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h, 0), color));

    // Bara de sus
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h - t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h - t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h, 0), color));

    // Linie verticală dreapta sus
    vertices.push_back(VertexFormat(origin + glm::vec3(w - t, h / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w - t, h, 0), color));

    // Bara din mijloc
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h / 2 - t / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h / 2 - t / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h / 2 + t / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h / 2 + t / 2, 0), color));

    // Linie diagonală (picior)
    vertices.push_back(VertexFormat(origin + glm::vec3(w / 2 - t / 2, h / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w / 2 + t / 2, h / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w - t, 0, 0), color));

    for (int i = 0; i < 5; i++) {
        int base = i * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);
    return mesh;
}

Mesh* object2D::CreateLetterE(
    const std::string& name,
    glm::vec3 origin,
    float size,
    glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float w = size * 0.6f;
    float h = size;
    float t = size * 0.15f;

    // Linie verticală stânga
    vertices.push_back(VertexFormat(origin + glm::vec3(0, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(t, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(t, h, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h, 0), color));

    // Bara de sus
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h - t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h - t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h, 0), color));

    // Bara din mijloc
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h / 2 - t / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w * 0.8f, h / 2 - t / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w * 0.8f, h / 2 + t / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h / 2 + t / 2, 0), color));

    // Bara de jos
    vertices.push_back(VertexFormat(origin + glm::vec3(0, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, t, 0), color));

    for (int i = 0; i < 4; i++) {
        int base = i * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);
    return mesh;
}

Mesh* object2D::CreateDots(
    const std::string& name,
    glm::vec3 origin,
    float size,
    glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float h = size;
    float dotSize = size * 0.2f;  // mărimea unui punct

    // Punct de sus (la 2/3 din înălțime)
    float topY = h * 0.66f;
    vertices.push_back(VertexFormat(origin + glm::vec3(0, topY, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(dotSize, topY, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(dotSize, topY + dotSize, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, topY + dotSize, 0), color));

    // Punct de jos (la 1/3 din înălțime)
    float bottomY = h * 0.33f - dotSize;
    vertices.push_back(VertexFormat(origin + glm::vec3(0, bottomY, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(dotSize, bottomY, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(dotSize, bottomY + dotSize, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, bottomY + dotSize, 0), color));

    // Indici pentru 2 pătrate
    for (int i = 0; i < 2; i++) {
        int base = i * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);
    return mesh;
}

Mesh* object2D::CreateDigit0(
    const std::string& name,
    glm::vec3 origin,
    float size,
    glm::vec3 color)
{
    // NEW: digit '0' implemented as an elliptical ring (outer ellipse - inner ellipse).
    // This makes it visually distinct from letter 'O' (which is rectangular style).
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float w = size * 0.6f;
    float h = size;
    float t = size * 0.15f; // thickness of the ring
    int numSegments = 40;

    glm::vec3 center = origin + glm::vec3(w * 0.5f, h * 0.5f, 0.0f);
    float outerRx = w * 0.5f;
    float outerRy = h * 0.5f;
    float innerRx = std::max(0.0f, outerRx - t);
    float innerRy = std::max(0.0f, outerRy - t);

    // outer ellipse vertices
    for (int i = 0; i < numSegments; ++i) {
        float angle = 2.0f * M_PI * i / numSegments;
        float x = center.x + outerRx * cos(angle);
        float y = center.y + outerRy * sin(angle);
        vertices.push_back(VertexFormat(glm::vec3(x, y, center.z), color));
    }

    // inner ellipse vertices (clockwise, same indexing offset)
    for (int i = 0; i < numSegments; ++i) {
        float angle = 2.0f * M_PI * i / numSegments;
        float x = center.x + innerRx * cos(angle);
        float y = center.y + innerRy * sin(angle);
        vertices.push_back(VertexFormat(glm::vec3(x, y, center.z), color));
    }

    // build quads (two triangles each) between outer and inner per segment
    for (int i = 0; i < numSegments; ++i) {
        int o0 = i;
        int o1 = (i + 1) % numSegments;
        int i0 = numSegments + i;
        int i1 = numSegments + ((i + 1) % numSegments);

        // triangle 1: o0, i0, i1
        indices.push_back(o0);
        indices.push_back(i0);
        indices.push_back(i1);

        // triangle 2: o0, i1, o1
        indices.push_back(o0);
        indices.push_back(i1);
        indices.push_back(o1);
    }

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);
    return mesh;
}

Mesh* object2D::CreateDigit1(
    const std::string& name,
    glm::vec3 origin,
    float size,
    glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float w = size * 0.6f;
    float h = size;
    float t = size * 0.15f; // thickness

    // MAIN vertical stem (we compute xPos early so the slanted bar can align to it)
    float xPos = w * 0.5f;

    // downward offset for the entire oblique bar (translate it a bit down)
    float slantedOffsetY = size * 0.1f; // tweak this value to move more/less down

    // 1) Oblique top bar (parallelogram rotated) - longer, starts at the top of the vertical stem
    float L = size * 0.45f;                // length of the slanted bar
    float angle = M_PI / 4.0f;             // 45 degrees
    // direction points left and down
    glm::vec2 dir = glm::vec2(-cos(angle), -sin(angle));
    glm::vec2 perp = glm::vec2(-dir.y, dir.x); // perpendicular for thickness
    float halfT = t * 0.5f;

    // place the slanted bar so its inner end starts from the top edge center of the vertical stem
    glm::vec2 p0 = glm::vec2(origin.x + xPos + t * 0.5f, origin.y + h); // start on top edge of stem
    // translate whole slanted bar down by offset
    p0 += glm::vec2(0.0f, -slantedOffsetY);
    // tiny inward overlap along dir to avoid visible seam with the stem
    p0 += dir * (-0.02f * size);

    glm::vec2 p1 = p0 + dir * L; // end of slanted bar (left-bottom)

    glm::vec2 v0 = p0 + perp * halfT;
    glm::vec2 v1 = p0 - perp * halfT;
    glm::vec2 v2 = p1 - perp * halfT;
    glm::vec2 v3 = p1 + perp * halfT;

    vertices.push_back(VertexFormat(glm::vec3(v0.x, v0.y, origin.z), color));
    vertices.push_back(VertexFormat(glm::vec3(v1.x, v1.y, origin.z), color));
    vertices.push_back(VertexFormat(glm::vec3(v2.x, v2.y, origin.z), color));
    vertices.push_back(VertexFormat(glm::vec3(v3.x, v3.y, origin.z), color));

    // 2) Main vertical stem (centered-ish)
    vertices.push_back(VertexFormat(origin + glm::vec3(xPos, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(xPos + t, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(xPos + t, h, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(xPos, h, 0), color));

    // 3) Base rectangle (to stabilize the bottom)
    vertices.push_back(VertexFormat(origin + glm::vec3(xPos - t, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(xPos + 2 * t, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(xPos + 2 * t, t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(xPos - t, t, 0), color));

    // Now create indices for all rectangles/parallelograms (each 4 vertices -> 2 triangles)
    int rectCount = (int)vertices.size() / 4;
    for (int i = 0; i < rectCount; ++i) {
        int base = i * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);
    return mesh;
}

Mesh* object2D::CreateDigit2(
    const std::string& name,
    glm::vec3 origin,
    float size,
    glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float w = size * 0.6f;
    float h = size;
    float t = size * 0.15f;

    // Bara de sus
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h - t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h - t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h, 0), color));

    // Linie verticală dreapta sus
    vertices.push_back(VertexFormat(origin + glm::vec3(w - t, h / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w - t, h, 0), color));

    // Bara din mijloc
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h / 2 - t / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h / 2 - t / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, h / 2 + t / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h / 2 + t / 2, 0), color));

    // Linie verticală stânga jos
    vertices.push_back(VertexFormat(origin + glm::vec3(0, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(t, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(t, h / 2, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, h / 2, 0), color));

    // Bara de jos
    vertices.push_back(VertexFormat(origin + glm::vec3(0, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, 0, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(w, t, 0), color));
    vertices.push_back(VertexFormat(origin + glm::vec3(0, t, 0), color));

    for (int i = 0; i < 5; i++) {
        int base = i * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);
    return mesh;
}
