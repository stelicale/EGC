#include "object3D.hpp"

#include <vector>
#include <cmath>

#include "core/engine.h"
#include "utils/gl_utils.h"

// 2D

Mesh *object3D::CreateCircle(const std::string &name, glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Circle center
    vertices.emplace_back(glm::vec3(0, 0, 0), color);

    // Generate points on the circumference
    int numSegments = 32;
    float radius = 1.0f;

    for (int i = 0; i <= numSegments; i++)
    {
        float theta = 2.0f * 3.14159f * float(i) / float(numSegments);
        float x = radius * cosf(theta);
        float y = radius * sinf(theta); // Build in the XY plane

        vertices.emplace_back(glm::vec3(x, y, 0), color);

        // Create indices for GL_TRIANGLE_FAN
        if (i < numSegments)
        {
            indices.push_back(0);     // Center
            indices.push_back(i + 1); // Current point
            indices.push_back(i + 2); // Next point
        }
    }

    Mesh *circle = new Mesh(name);
    circle->InitFromData(vertices, indices);
    return circle;
}

Mesh *object3D::CreateTriangle(const std::string &name, glm::vec3 color)
{
    // Function to generate a TRIANGLE (Green - for pickup)
    std::vector<VertexFormat> vertices =
        {
            VertexFormat(glm::vec3(0, 0.5f, 0), color),      // Top vertex
            VertexFormat(glm::vec3(-0.5f, -0.5f, 0), color), // Bottom left
            VertexFormat(glm::vec3(0.5f, -0.5f, 0), color)   // Bottom right
        };

    std::vector<unsigned int> indices = {0, 1, 2};

    Mesh *triangle = new Mesh(name);
    triangle->InitFromData(vertices, indices);
    return triangle;
}

Mesh *object3D::CreateArrow(const std::string &name, glm::vec3 color)
{
    // Function to generate an ARROW (White - for player/camera)
    std::vector<VertexFormat> vertices =
        {
            // Arrow tip
            VertexFormat(glm::vec3(0, 0.5f, 0), color),
            VertexFormat(glm::vec3(-0.3f, 0.0f, 0), color),
            VertexFormat(glm::vec3(0.3f, 0.0f, 0), color),

            // Arrow tail
            VertexFormat(glm::vec3(-0.1f, 0.0f, 0), color),
            VertexFormat(glm::vec3(0.1f, 0.0f, 0), color),
            VertexFormat(glm::vec3(-0.1f, -0.5f, 0), color),
            VertexFormat(glm::vec3(0.1f, -0.5f, 0), color)};

    std::vector<unsigned int> indices =
        {
            0, 1, 2,         // Tip triangle
            3, 4, 6, 3, 6, 5 // Tail rectangle
        };

    Mesh *arrow = new Mesh(name);
    arrow->InitFromData(vertices, indices);
    return arrow;
}

Mesh *object3D::CreateRectangle(
    const std::string &name,
    float length,
    float height,
    glm::vec3 color)
{
    std::vector<VertexFormat> vertices =
        {
            VertexFormat(glm::vec3(-length / 2, -height / 2, 0), color),
            VertexFormat(glm::vec3(length / 2, -height / 2, 0), color),
            VertexFormat(glm::vec3(length / 2, height / 2, 0), color),
            VertexFormat(glm::vec3(-length / 2, height / 2, 0), color)};

    std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

    Mesh *rectangle = new Mesh(name);
    rectangle->InitFromData(vertices, indices);
    return rectangle;
}

Mesh *object3D::CreateStar(const std::string &name, glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    vertices.emplace_back(glm::vec3(0, 0, 0), color); // Center

    float rOuter = 1.0f;
    float rInner = 0.4f;
    int numPoints = 5;

    for (int i = 0; i <= numPoints * 2; i++)
    {
        // Alternate radii (outer, inner, outer...)
        float radius = (i % 2 == 0) ? rOuter : rInner;
        // Current angle: 360 degrees / (2 * 5 points) = 36 degrees per step
        float currAngle = (i * 3.14159f) / numPoints; // PI / 5

        // Rotate by PI/2 (90 degrees) so the star stands "upright" (tip pointing up)
        float x = radius * cosf(currAngle + 1.5708f);
        float y = radius * sinf(currAngle + 1.5708f);

        vertices.emplace_back(glm::vec3(x, y, 0), color);

        if (i < numPoints * 2)
        {
            indices.push_back(0);
            indices.push_back(i + 1);
            indices.push_back(i + 2);
        }
    }

    Mesh *star = new Mesh(name);
    star->InitFromData(vertices, indices);
    return star;
}

Mesh *object3D::CreateSquare(const std::string &name, glm::vec3 color)
{
    // Function to generate a SQUARE (used for obstacles/train)
    std::vector<VertexFormat> vertices =
        {
            VertexFormat(glm::vec3(-0.5f, -0.5f, 0), color),
            VertexFormat(glm::vec3(0.5f, -0.5f, 0), color),
            VertexFormat(glm::vec3(0.5f, 0.5f, 0), color),
            VertexFormat(glm::vec3(-0.5f, 0.5f, 0), color)};

    std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

    Mesh *square = new Mesh(name);
    square->InitFromData(vertices, indices);
    return square;
}

void AddBox(std::vector<VertexFormat> &vertices, std::vector<unsigned int> &indices,
            glm::vec3 minPoint, glm::vec3 maxPoint, glm::vec3 color, bool hasBlackBorder = false)
{
    // Internal helper to add a cube (parallelepiped) to the vertex and index lists
    // This function reduces code duplication

    // Define the 8 corners of the box
    glm::vec3 p0 = glm::vec3(minPoint.x, minPoint.y, maxPoint.z); // Left-Bottom-Front
    glm::vec3 p1 = glm::vec3(maxPoint.x, minPoint.y, maxPoint.z); // Right-Bottom-Front
    glm::vec3 p2 = glm::vec3(maxPoint.x, maxPoint.y, maxPoint.z); // Right-Top-Front
    glm::vec3 p3 = glm::vec3(minPoint.x, maxPoint.y, maxPoint.z); // Left-Top-Front
    glm::vec3 p4 = glm::vec3(minPoint.x, minPoint.y, minPoint.z); // Left-Bottom-Back
    glm::vec3 p5 = glm::vec3(maxPoint.x, minPoint.y, minPoint.z); // Right-Bottom-Back
    glm::vec3 p6 = glm::vec3(maxPoint.x, maxPoint.y, minPoint.z); // Right-Top-Back
    glm::vec3 p7 = glm::vec3(minPoint.x, maxPoint.y, minPoint.z); // Left-Top-Back

    glm::vec3 black = glm::vec3(0.0f, 0.0f, 0.0f);
    float thickness = 0.02f; // Fixed outline thickness (adjust if needed)

    auto CreateFace = [&](glm::vec3 A, glm::vec3 B, glm::vec3 C, glm::vec3 D, glm::vec3 faceColor)
    {
        unsigned int faceStart = vertices.size();

        if (hasBlackBorder)
        {
            // Calculate directions to obtain inner points at a fixed distance
            // For axis-aligned rectangles, directions are simple unit vectors.
            // A -> B is one side, A -> D is the other.

            glm::vec3 AB = glm::normalize(B - A);
            glm::vec3 AD = glm::normalize(D - A);
            glm::vec3 CB = glm::normalize(B - C);
            glm::vec3 CD = glm::normalize(D - C);

            // Calculate inner corners by translating with 'thickness' in both directions
            glm::vec3 iA = A + (AB + AD) * thickness;
            glm::vec3 iB = B + (glm::normalize(A - B) + glm::normalize(C - B)) * thickness;
            glm::vec3 iC = C + (CB + CD) * thickness;
            glm::vec3 iD = D + (glm::normalize(A - D) + glm::normalize(C - D)) * thickness;

            // 1. COLORED CORE (4 inner vertices)
            vertices.push_back(VertexFormat(iA, faceColor)); // 0
            vertices.push_back(VertexFormat(iB, faceColor)); // 1
            vertices.push_back(VertexFormat(iC, faceColor)); // 2
            vertices.push_back(VertexFormat(iD, faceColor)); // 3

            indices.push_back(faceStart + 0);
            indices.push_back(faceStart + 1);
            indices.push_back(faceStart + 2);
            indices.push_back(faceStart + 0);
            indices.push_back(faceStart + 2);
            indices.push_back(faceStart + 3);

            // 2. BLACK FRAME (8 vertices: 4 outer + 4 inner duplicated as black)
            unsigned int frameStart = vertices.size();

            // Exterior (Black)
            vertices.push_back(VertexFormat(A, black)); // 0
            vertices.push_back(VertexFormat(B, black)); // 1
            vertices.push_back(VertexFormat(C, black)); // 2
            vertices.push_back(VertexFormat(D, black)); // 3

            // Interior (Black) - spatially coincides with iA, iB...
            vertices.push_back(VertexFormat(iA, black)); // 4
            vertices.push_back(VertexFormat(iB, black)); // 5
            vertices.push_back(VertexFormat(iC, black)); // 6
            vertices.push_back(VertexFormat(iD, black)); // 7

            // Frame triangles
            // Bottom
            indices.push_back(frameStart + 0);
            indices.push_back(frameStart + 1);
            indices.push_back(frameStart + 5);
            indices.push_back(frameStart + 0);
            indices.push_back(frameStart + 5);
            indices.push_back(frameStart + 4);
            // Right
            indices.push_back(frameStart + 1);
            indices.push_back(frameStart + 2);
            indices.push_back(frameStart + 6);
            indices.push_back(frameStart + 1);
            indices.push_back(frameStart + 6);
            indices.push_back(frameStart + 5);
            // Top
            indices.push_back(frameStart + 2);
            indices.push_back(frameStart + 3);
            indices.push_back(frameStart + 7);
            indices.push_back(frameStart + 2);
            indices.push_back(frameStart + 7);
            indices.push_back(frameStart + 6);
            // Left
            indices.push_back(frameStart + 3);
            indices.push_back(frameStart + 0);
            indices.push_back(frameStart + 4);
            indices.push_back(frameStart + 3);
            indices.push_back(frameStart + 4);
            indices.push_back(frameStart + 7);
        }
        else
        {
            // Standard (no border)
            vertices.push_back(VertexFormat(A, faceColor));
            vertices.push_back(VertexFormat(B, faceColor));
            vertices.push_back(VertexFormat(C, faceColor));
            vertices.push_back(VertexFormat(D, faceColor));

            indices.push_back(faceStart + 0);
            indices.push_back(faceStart + 1);
            indices.push_back(faceStart + 2);
            indices.push_back(faceStart + 0);
            indices.push_back(faceStart + 2);
            indices.push_back(faceStart + 3);
        }
    };

    // Build the 6 faces
    CreateFace(p0, p1, p2, p3, color); // Front
    CreateFace(p5, p4, p7, p6, color); // Back
    CreateFace(p4, p0, p3, p7, color); // Left
    CreateFace(p1, p5, p6, p2, color); // Right
    CreateFace(p3, p2, p6, p7, color); // Top
    CreateFace(p4, p5, p1, p0, color); // Bottom
}

Mesh *object3D::CreateGround(
    const std::string &name,
    float length,
    glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float halfLength = length / 2.0f;
    // Place it slightly under 0 (-0.01f) so it doesn't overlap uglily with rails at 0
    float yPos = -0.01f;

    // 4 vertices forming a large square in the XZ plane
    vertices.push_back(VertexFormat(glm::vec3(-halfLength, yPos, -halfLength), color)); // 0
    vertices.push_back(VertexFormat(glm::vec3(halfLength, yPos, -halfLength), color));  // 1
    vertices.push_back(VertexFormat(glm::vec3(halfLength, yPos, halfLength), color));   // 2
    vertices.push_back(VertexFormat(glm::vec3(-halfLength, yPos, halfLength), color));  // 3

    // Two triangles
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);

    Mesh *ground = new Mesh(name);
    ground->InitFromData(vertices, indices);
    return ground;
}

Mesh *object3D::CreateWater(
    const std::string &name,
    float width, float height) // height here represents length on Z axis
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 waterColor(0.0f, 0.5f, 0.8f); // Blue

    // Water is a flat rectangle (very thin) on the ground
    // Centered at origin on X and Z, at a very low y height under the rails
    float yPos = 0.0f;
    float thicknessY = 0.001f; // Very thin, practically flat

    glm::vec3 minPoint(-width / 2.0f, yPos, -height / 2.0f);
    glm::vec3 maxPoint(width / 2.0f, yPos + thicknessY, height / 2.0f);

    // Use AddBox with hasBlackBorder = true to have an outline
    AddBox(vertices, indices, minPoint, maxPoint, waterColor, false);

    Mesh *water = new Mesh(name);
    water->InitFromData(vertices, indices);
    return water;
}

Mesh *object3D::CreateMountain(
    const std::string &name,
    float width, float height) // height here represents peak height on Y
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 mountainColor(0.55f, 0.45f, 0.35f); // Brown

    // The mountain is a triangle in the XY plane
    // Centered at origin on X and Y, at a very low y height under the rails
    float yPos = 0.0f;
    float thicknessY = 0.001f; // Very thin, practically flat

    glm::vec3 minPoint(-width / 2.0f, yPos, -height / 2.0f);
    glm::vec3 maxPoint(width / 2.0f, yPos + thicknessY, height / 2.0f);

    // Use AddBox with hasBlackBorder = true to have an outline
    AddBox(vertices, indices, minPoint, maxPoint, mountainColor, false);

    Mesh *mountain = new Mesh(name);
    mountain->InitFromData(vertices, indices);
    return mountain;
}

void AddCylinder(std::vector<VertexFormat> &vertices, std::vector<unsigned int> &indices,
                 glm::vec3 center, float radius, float height, glm::vec3 color,
                 int segments = 20, bool axisX = false, bool hasColoredSide = true)
{
    unsigned int start = vertices.size();
    glm::vec3 black = glm::vec3(0.0f, 0.0f, 0.0f);
    float thickness = 0.02f;

    // SIDE PART (MANTLE)
    unsigned int sideStart = vertices.size();

    // Lambda to calculate the position of a point on the circle
    auto GetPoint = [&](float hOffset, float angle) -> glm::vec3
    {
        float cosA = cos(angle);
        float sinA = sin(angle);
        if (axisX)
            return glm::vec3(center.x + hOffset, center.y + radius * cosA, center.z + radius * sinA);
        return glm::vec3(center.x + radius * cosA, center.y + radius * sinA, center.z + hOffset);
    };

    if (hasColoredSide)
    {
        // CASE A: With colored band - Banded cylinder (black-color-black)
        float hStart = axisX ? -height / 2 : -height / 2;
        float hEnd = axisX ? height / 2 : height / 2;
        float hInnerStart = hStart + thickness;
        float hInnerEnd = hEnd - thickness;

        // Generate rings of vertices
        // 0: Start (Black), 1: InnerStart (Black), 2: InnerStart (Color), 3: InnerEnd (Color), 4: InnerEnd (Black), 5: End (Black)
        for (int i = 0; i <= segments; i++)
        {
            float angle = 2.0f * M_PI * i / segments;
            vertices.push_back(VertexFormat(GetPoint(hStart, angle), black));      // 0
            vertices.push_back(VertexFormat(GetPoint(hInnerStart, angle), black)); // 1
            vertices.push_back(VertexFormat(GetPoint(hInnerStart, angle), color)); // 2
            vertices.push_back(VertexFormat(GetPoint(hInnerEnd, angle), color));   // 3
            vertices.push_back(VertexFormat(GetPoint(hInnerEnd, angle), black));   // 4
            vertices.push_back(VertexFormat(GetPoint(hEnd, angle), black));        // 5
        }

        // Indices (3 sets of quads)
        for (int i = 0; i < segments; i++)
        {
            int base = i * 6;
            int next = (i + 1) * 6;

            // 1. Black Start Band (0 -> 1)
            indices.push_back(sideStart + base + 0);
            indices.push_back(sideStart + next + 0);
            indices.push_back(sideStart + base + 1);
            indices.push_back(sideStart + base + 1);
            indices.push_back(sideStart + next + 0);
            indices.push_back(sideStart + next + 1);

            // 2. Colored Middle Band (2 -> 3)
            indices.push_back(sideStart + base + 2);
            indices.push_back(sideStart + next + 2);
            indices.push_back(sideStart + base + 3);
            indices.push_back(sideStart + base + 3);
            indices.push_back(sideStart + next + 2);
            indices.push_back(sideStart + next + 3);

            // 3. Black End Band (4 -> 5)
            indices.push_back(sideStart + base + 4);
            indices.push_back(sideStart + next + 4);
            indices.push_back(sideStart + base + 5);
            indices.push_back(sideStart + base + 5);
            indices.push_back(sideStart + next + 4);
            indices.push_back(sideStart + next + 5);
        }
    }
    else
    {
        // CASE B: Without colored band - Completely black lateral cylinder
        for (int i = 0; i <= segments; i++)
        {
            float angle = 2.0f * M_PI * i / segments;
            vertices.push_back(VertexFormat(GetPoint(axisX ? -height / 2 : -height / 2, angle), black));
            vertices.push_back(VertexFormat(GetPoint(axisX ? height / 2 : height / 2, angle), black));
        }
        // Indices (one set of quads)
        for (int i = 0; i < segments; i++)
        {
            int curr = i * 2;
            int next = (i + 1) * 2;
            indices.push_back(sideStart + curr);
            indices.push_back(sideStart + next);
            indices.push_back(sideStart + curr + 1);
            indices.push_back(sideStart + curr + 1);
            indices.push_back(sideStart + next);
            indices.push_back(sideStart + next + 1);
        }
    }

    // CAPS
    auto CreateCap = [&](bool isFront)
    {
        float zPos = isFront ? height / 2 : -height / 2;
        float xOffset = axisX ? zPos : 0.0f;
        float zOffset = axisX ? 0.0f : zPos;

        // Caps always have a black outline + colored inner circle
        float innerRadius = radius - thickness;
        if (innerRadius < 0)
            innerRadius = 0;

        // A. OUTER RING (Black Outline)
        unsigned int ringStart = vertices.size();
        for (int i = 0; i <= segments; i++)
        {
            float angle = 2.0f * M_PI * i / segments;
            float cosA = cos(angle);
            float sinA = sin(angle);

            glm::vec3 pOut, pIn;
            if (axisX)
            {
                pOut = glm::vec3(center.x + xOffset, center.y + radius * cosA, center.z + radius * sinA);
                pIn = glm::vec3(center.x + xOffset, center.y + innerRadius * cosA, center.z + innerRadius * sinA);
            }
            else
            {
                pOut = glm::vec3(center.x + radius * cosA, center.y + radius * sinA, center.z + zOffset);
                pIn = glm::vec3(center.x + innerRadius * cosA, center.y + innerRadius * sinA, center.z + zOffset);
            }
            vertices.push_back(VertexFormat(pOut, black)); // Outer
            vertices.push_back(VertexFormat(pIn, black));  // Inner
        }

        for (int i = 0; i < segments; i++)
        {
            int curr = i * 2;
            int next = (i + 1) * 2;
            if (isFront)
            {
                indices.push_back(ringStart + curr);
                indices.push_back(ringStart + next);
                indices.push_back(ringStart + curr + 1);
                indices.push_back(ringStart + curr + 1);
                indices.push_back(ringStart + next);
                indices.push_back(ringStart + next + 1);
            }
            else
            {
                indices.push_back(ringStart + curr);
                indices.push_back(ringStart + curr + 1);
                indices.push_back(ringStart + next);
                indices.push_back(ringStart + curr + 1);
                indices.push_back(ringStart + next + 1);
                indices.push_back(ringStart + next);
            }
        }

        // B. INNER CIRCLE (Colored)
        unsigned int fanStart = vertices.size();
        glm::vec3 cPos = axisX ? glm::vec3(center.x + xOffset, center.y, center.z)
                               : glm::vec3(center.x, center.y, center.z + zOffset);
        vertices.push_back(VertexFormat(cPos, color));

        for (int i = 0; i <= segments; i++)
        {
            float angle = 2.0f * M_PI * i / segments;
            float cosA = cos(angle);
            float sinA = sin(angle);
            glm::vec3 p;
            if (axisX)
                p = glm::vec3(center.x + xOffset, center.y + innerRadius * cosA, center.z + innerRadius * sinA);
            else
                p = glm::vec3(center.x + innerRadius * cosA, center.y + innerRadius * sinA, center.z + zOffset);
            vertices.push_back(VertexFormat(p, color));
        }

        for (int i = 0; i < segments; i++)
        {
            if (isFront)
            {
                indices.push_back(fanStart + 0);
                indices.push_back(fanStart + 1 + i);
                indices.push_back(fanStart + 1 + i + 1);
            }
            else
            {
                indices.push_back(fanStart + 0);
                indices.push_back(fanStart + 1 + i + 1);
                indices.push_back(fanStart + 1 + i);
            }
        }
    };

    CreateCap(false);
    CreateCap(true);
}

void AddPyramid(std::vector<VertexFormat> &vertices, std::vector<unsigned int> &indices,
                glm::vec3 baseCenter, float baseSize, float height, glm::vec3 color, bool hasBlackBorder = false)
{
    glm::vec3 black = glm::vec3(0.0f, 0.0f, 0.0f);
    float thickness = 0.02f;

    // Calculate the 4 base corners and the pyramid apex
    float halfSize = baseSize / 2.0f;
    glm::vec3 b0 = baseCenter + glm::vec3(-halfSize, 0.0f, -halfSize); // Left-Back
    glm::vec3 b1 = baseCenter + glm::vec3(halfSize, 0.0f, -halfSize);  // Right-Back
    glm::vec3 b2 = baseCenter + glm::vec3(halfSize, 0.0f, halfSize);   // Right-Front
    glm::vec3 b3 = baseCenter + glm::vec3(-halfSize, 0.0f, halfSize);  // Left-Front
    glm::vec3 apex = baseCenter + glm::vec3(0.0f, height, 0.0f);       // Apex

    if (!hasBlackBorder)
    {
        // Simple pyramid without outline
        unsigned int baseStart = vertices.size();

        // Base (square)
        vertices.push_back(VertexFormat(b0, color));
        vertices.push_back(VertexFormat(b1, color));
        vertices.push_back(VertexFormat(b2, color));
        vertices.push_back(VertexFormat(b3, color));

        indices.push_back(baseStart + 0);
        indices.push_back(baseStart + 1);
        indices.push_back(baseStart + 2);
        indices.push_back(baseStart + 0);
        indices.push_back(baseStart + 2);
        indices.push_back(baseStart + 3);

        // Apex
        vertices.push_back(VertexFormat(apex, color));
        unsigned int apexIdx = vertices.size() - 1;

        // The 4 side faces
        indices.push_back(baseStart + 0);
        indices.push_back(baseStart + 1);
        indices.push_back(apexIdx);
        indices.push_back(baseStart + 1);
        indices.push_back(baseStart + 2);
        indices.push_back(apexIdx);
        indices.push_back(baseStart + 2);
        indices.push_back(baseStart + 3);
        indices.push_back(apexIdx);
        indices.push_back(baseStart + 3);
        indices.push_back(baseStart + 0);
        indices.push_back(apexIdx);
    }
    else
    {
        // Pyramid with black outline

        // 1. BASE with outline
        unsigned int baseStart = vertices.size();

        // Inner base points (retract by thickness)
        glm::vec3 ib0 = b0 + glm::vec3(thickness, 0.0f, thickness);
        glm::vec3 ib1 = b1 + glm::vec3(-thickness, 0.0f, thickness);
        glm::vec3 ib2 = b2 + glm::vec3(-thickness, 0.0f, -thickness);
        glm::vec3 ib3 = b3 + glm::vec3(thickness, 0.0f, -thickness);

        // Colored core of the base
        vertices.push_back(VertexFormat(ib0, color)); // 0
        vertices.push_back(VertexFormat(ib1, color)); // 1
        vertices.push_back(VertexFormat(ib2, color)); // 2
        vertices.push_back(VertexFormat(ib3, color)); // 3

        indices.push_back(baseStart + 0);
        indices.push_back(baseStart + 1);
        indices.push_back(baseStart + 2);
        indices.push_back(baseStart + 0);
        indices.push_back(baseStart + 2);
        indices.push_back(baseStart + 3);

        // Black frame of the base
        unsigned int frameStart = vertices.size();

        // Exterior (Black)
        vertices.push_back(VertexFormat(b0, black)); // 0
        vertices.push_back(VertexFormat(b1, black)); // 1
        vertices.push_back(VertexFormat(b2, black)); // 2
        vertices.push_back(VertexFormat(b3, black)); // 3

        // Interior (Black)
        vertices.push_back(VertexFormat(ib0, black)); // 4
        vertices.push_back(VertexFormat(ib1, black)); // 5
        vertices.push_back(VertexFormat(ib2, black)); // 6
        vertices.push_back(VertexFormat(ib3, black)); // 7

        // Base frame triangles
        // Back (0-1)
        indices.push_back(frameStart + 0);
        indices.push_back(frameStart + 1);
        indices.push_back(frameStart + 5);
        indices.push_back(frameStart + 0);
        indices.push_back(frameStart + 5);
        indices.push_back(frameStart + 4);
        // Right (1-2)
        indices.push_back(frameStart + 1);
        indices.push_back(frameStart + 2);
        indices.push_back(frameStart + 6);
        indices.push_back(frameStart + 1);
        indices.push_back(frameStart + 6);
        indices.push_back(frameStart + 5);
        // Front (2-3)
        indices.push_back(frameStart + 2);
        indices.push_back(frameStart + 3);
        indices.push_back(frameStart + 7);
        indices.push_back(frameStart + 2);
        indices.push_back(frameStart + 7);
        indices.push_back(frameStart + 6);
        // Left (3-0)
        indices.push_back(frameStart + 3);
        indices.push_back(frameStart + 0);
        indices.push_back(frameStart + 4);
        indices.push_back(frameStart + 3);
        indices.push_back(frameStart + 4);
        indices.push_back(frameStart + 7);

        // 2. LATERAL FACES with outline
        // For each lateral face: inner colored triangle + black edges

        // Calculate inner apex (retracted vertically and horizontally)
        glm::vec3 iApex = apex - glm::vec3(0.0f, thickness * 2.0f, 0.0f);

        auto CreateTriangleFace = [&](glm::vec3 outerA, glm::vec3 outerB, glm::vec3 innerA, glm::vec3 innerB)
        {
            unsigned int faceStart = vertices.size();

            // Inner colored triangle
            vertices.push_back(VertexFormat(innerA, color)); // 0
            vertices.push_back(VertexFormat(innerB, color)); // 1
            vertices.push_back(VertexFormat(iApex, color));  // 2

            indices.push_back(faceStart + 0);
            indices.push_back(faceStart + 1);
            indices.push_back(faceStart + 2);

            // Black edges (3 trapezoidal bands)
            unsigned int edgeStart = vertices.size();

            // Base edge (outerA -> outerB)
            vertices.push_back(VertexFormat(outerA, black)); // 0
            vertices.push_back(VertexFormat(outerB, black)); // 1
            vertices.push_back(VertexFormat(innerB, black)); // 2
            vertices.push_back(VertexFormat(innerA, black)); // 3

            indices.push_back(edgeStart + 0);
            indices.push_back(edgeStart + 1);
            indices.push_back(edgeStart + 2);
            indices.push_back(edgeStart + 0);
            indices.push_back(edgeStart + 2);
            indices.push_back(edgeStart + 3);

            // Left edge (outerA -> apex)
            unsigned int leftStart = vertices.size();
            vertices.push_back(VertexFormat(outerA, black)); // 0
            vertices.push_back(VertexFormat(apex, black));   // 1
            vertices.push_back(VertexFormat(iApex, black));  // 2
            vertices.push_back(VertexFormat(innerA, black)); // 3

            indices.push_back(leftStart + 0);
            indices.push_back(leftStart + 1);
            indices.push_back(leftStart + 2);
            indices.push_back(leftStart + 0);
            indices.push_back(leftStart + 2);
            indices.push_back(leftStart + 3);

            // Right edge (outerB -> apex)
            unsigned int rightStart = vertices.size();
            vertices.push_back(VertexFormat(outerB, black)); // 0
            vertices.push_back(VertexFormat(apex, black));   // 1
            vertices.push_back(VertexFormat(iApex, black));  // 2
            vertices.push_back(VertexFormat(innerB, black)); // 3

            indices.push_back(rightStart + 0);
            indices.push_back(rightStart + 1);
            indices.push_back(rightStart + 2);
            indices.push_back(rightStart + 0);
            indices.push_back(rightStart + 2);
            indices.push_back(rightStart + 3);
        };

        // The 4 lateral faces
        CreateTriangleFace(b0, b1, ib0, ib1); // Back
        CreateTriangleFace(b1, b2, ib1, ib2); // Right
        CreateTriangleFace(b2, b3, ib2, ib3); // Front
        CreateTriangleFace(b3, b0, ib3, ib0); // Left
    }
}

void AddSphere(std::vector<VertexFormat> &vertices, std::vector<unsigned int> &indices,
               glm::vec3 center, float radius, glm::vec3 color,
               int latitudeSegments, int longitudeSegments, bool hasBlackBorder = false)
{
    glm::vec3 black = glm::vec3(0.0f, 0.0f, 0.0f);
    float thickness = 0.02f;

    if (!hasBlackBorder)
    {
        // Simple sphere without outline
        unsigned int baseIdx = vertices.size();

        // Generate vertices
        for (int lat = 0; lat <= latitudeSegments; lat++)
        {
            float theta = lat * M_PI / latitudeSegments;
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            for (int lon = 0; lon <= longitudeSegments; lon++)
            {
                float phi = lon * 2 * M_PI / longitudeSegments;
                float sinPhi = sin(phi);
                float cosPhi = cos(phi);

                glm::vec3 pos = center + glm::vec3(
                                             radius * sinTheta * cosPhi,
                                             radius * cosTheta,
                                             radius * sinTheta * sinPhi);

                vertices.push_back(VertexFormat(pos, color));
            }
        }

        // Generate indices
        for (int lat = 0; lat < latitudeSegments; lat++)
        {
            for (int lon = 0; lon < longitudeSegments; lon++)
            {
                int current = baseIdx + lat * (longitudeSegments + 1) + lon;
                int next = current + longitudeSegments + 1;

                indices.push_back(current);
                indices.push_back(next);
                indices.push_back(current + 1);

                indices.push_back(current + 1);
                indices.push_back(next);
                indices.push_back(next + 1);
            }
        }
    }
    else
    {
        // Sphere with black outline
        float innerRadius = radius - thickness;
        if (innerRadius < 0)
            innerRadius = 0;

        unsigned int baseIdx = vertices.size();

        // Generate vertices for outer (black) and inner (colored) spheres
        for (int lat = 0; lat <= latitudeSegments; lat++)
        {
            float theta = lat * M_PI / latitudeSegments;
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            for (int lon = 0; lon <= longitudeSegments; lon++)
            {
                float phi = lon * 2 * M_PI / longitudeSegments;
                float sinPhi = sin(phi);
                float cosPhi = cos(phi);

                glm::vec3 direction = glm::vec3(
                    sinTheta * cosPhi,
                    cosTheta,
                    sinTheta * sinPhi);

                // Outer vertex (black)
                glm::vec3 outerPos = center + direction * radius;
                vertices.push_back(VertexFormat(outerPos, black));

                // Inner vertex (colored)
                glm::vec3 innerPos = center + direction * innerRadius;
                vertices.push_back(VertexFormat(innerPos, color));
            }
        }

        // Generate indices for outline band and colored surface
        for (int lat = 0; lat < latitudeSegments; lat++)
        {
            for (int lon = 0; lon < longitudeSegments; lon++)
            {
                int current = baseIdx + lat * (longitudeSegments + 1) * 2 + lon * 2;
                int next = current + (longitudeSegments + 1) * 2;

                // Outline band (between outer and inner)
                // Quad: exterior_current -> exterior_next -> interior_next -> interior_current
                indices.push_back(current);  // exterior current
                indices.push_back(next);     // exterior next
                indices.push_back(next + 1); // interior next

                indices.push_back(current);     // exterior current
                indices.push_back(next + 1);    // interior next
                indices.push_back(current + 1); // interior current

                // Inner colored surface
                indices.push_back(current + 1);
                indices.push_back(next + 1);
                indices.push_back(current + 3);

                indices.push_back(current + 3);
                indices.push_back(next + 1);
                indices.push_back(next + 3);
            }
        }
    }
}

// Cylinder oriented on Y axis, uniformly colored
void AddCylinderY(std::vector<VertexFormat> &vertices, std::vector<unsigned int> &indices,
                  glm::vec3 baseCenter, float radius, float height, glm::vec3 color,
                  int segments = 24)
{
    unsigned int start = vertices.size();

    for (int i = 0; i < segments; i++)
    {
        float angle = 2.0f * M_PI * i / segments;
        float cosA = cos(angle);
        float sinA = sin(angle);

        glm::vec3 bottom = baseCenter + glm::vec3(radius * cosA, 0.0f, radius * sinA);
        glm::vec3 top = bottom + glm::vec3(0.0f, height, 0.0f);

        vertices.push_back(VertexFormat(bottom, color)); // 0
        vertices.push_back(VertexFormat(top, color));    // 1
    }

    for (int i = 0; i < segments; i++)
    {
        int curr = start + i * 2;
        int next = start + ((i + 1) % segments) * 2;

        // Side face (two triangles)
        indices.push_back(curr);
        indices.push_back(next);
        indices.push_back(curr + 1);

        indices.push_back(curr + 1);
        indices.push_back(next);
        indices.push_back(next + 1);
    }

    // Caps
    unsigned int centerBottom = vertices.size();
    vertices.push_back(VertexFormat(baseCenter, color));
    unsigned int centerTop = vertices.size();
    vertices.push_back(VertexFormat(baseCenter + glm::vec3(0.0f, height, 0.0f), color));

    for (int i = 0; i < segments; i++)
    {
        unsigned int curr = start + i * 2;
        unsigned int next = start + ((i + 1) % segments) * 2;

        // Base
        indices.push_back(centerBottom);
        indices.push_back(next);
        indices.push_back(curr);

        // Top Cap
        indices.push_back(centerTop);
        indices.push_back(curr + 1);
        indices.push_back(next + 1);
    }
}

// Cone oriented on Y axis (base in XZ plane, apex in +Y)
void AddCone(std::vector<VertexFormat> &vertices, std::vector<unsigned int> &indices,
             glm::vec3 baseCenter, float radius, float height, glm::vec3 color,
             int segments = 24)
{
    glm::vec3 baseColor = color * 0.7f; // base slightly darker than cone
    unsigned int baseStart = vertices.size();

    for (int i = 0; i < segments; i++)
    {
        float angle = 2.0f * M_PI * i / segments;
        float cosA = cos(angle);
        float sinA = sin(angle);
        glm::vec3 p = baseCenter + glm::vec3(radius * cosA, 0.0f, radius * sinA);
        vertices.push_back(VertexFormat(p, baseColor));
    }

    // Apex
    unsigned int apexIndex = vertices.size();
    vertices.push_back(VertexFormat(baseCenter + glm::vec3(0.0f, height, 0.0f), color));

    // Sides
    for (int i = 0; i < segments; i++)
    {
        unsigned int curr = baseStart + i;
        unsigned int next = baseStart + (i + 1) % segments;
        indices.push_back(curr);
        indices.push_back(next);
        indices.push_back(apexIndex);
    }

    // Base Center
    unsigned int centerIndex = vertices.size();
    vertices.push_back(VertexFormat(baseCenter, baseColor));

    for (int i = 0; i < segments; i++)
    {
        unsigned int curr = baseStart + i;
        unsigned int next = baseStart + (i + 1) % segments;
        indices.push_back(centerIndex);
        indices.push_back(next);
        indices.push_back(curr);
    }
}

Mesh *object3D::CreateTree(
    const std::string &name,
    glm::vec3 trunkColor,
    glm::vec3 leavesColor)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Trunk
    float trunkHeight = 0.6f;
    float trunkRadius = 0.12f;
    AddCylinderY(vertices, indices, glm::vec3(0.0f), trunkRadius, trunkHeight, trunkColor, 24);

    // Leaves: 3 identical cones, slightly offset on Y
    float coneHeight = 0.6f;
    float coneRadius = 0.6f;
    float offset = 0.25f;
    glm::vec3 baseCenter(0.0f, trunkHeight, 0.0f);

    for (int i = 0; i < 3; i++)
    {
        glm::vec3 coneBase = baseCenter + glm::vec3(0.0f, i * offset, 0.0f);
        AddCone(vertices, indices, coneBase, coneRadius, coneHeight, leavesColor, 24);
    }

    Mesh *tree = new Mesh(name);
    tree->InitFromData(vertices, indices);
    return tree;
}

Mesh *object3D::CreateLocomotive(
    const std::string &name,
    glm::vec3 bodyColor,
    glm::vec3 cabinColor,
    glm::vec3 wheelColor)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 baseColor(1.0f, 0.8f, 0.0f); // Yellow
    glm::vec3 pinkColor(1.0f, 0.4f, 0.7f); // Pink

    // 1. Base (Yellow) - Platform
    // Width (Z) reduced to +/- 0.4.
    AddBox(vertices, indices, glm::vec3(-1.0f, 0.30f, -0.4f), glm::vec3(1.0f, 0.45f, 0.4f), baseColor, true);

    // 2. Cabin (Green) - In the back
    // Sits on the base (starts from 0.45)
    AddBox(vertices, indices, glm::vec3(-1.0f, 0.45f, -0.4f), glm::vec3(-0.2f, 1.15f, 0.4f), bodyColor, true);

    // 3. Engine (Blue) - CYLINDER oriented on X axis
    AddCylinder(vertices, indices, glm::vec3(0.35f, 0.65f, 0.0f), 0.2f, 1.1f, cabinColor, 20, true, true);

    // 5. Connector (Pink) - Small CYLINDER attached to the engine end
    AddCylinder(vertices, indices, glm::vec3(0.96f, 0.65f, 0.0f), 0.08f, 0.12f, pinkColor, 20, true, true);

    // 6. Wheels (Red) - CYLINDERS oriented on Z axis
    float wheelY = 0.15f;
    float wheelRadius = 0.15f;
    float wheelThickness = 0.12f;
    float zOffset = 0.3f; // Set according to new requirement

    for (float x = -0.7f; x <= 0.7f; x += 0.35f)
    {
        // Left Wheel - hasBlackTread = true
        AddCylinder(vertices, indices, glm::vec3(x, wheelY, zOffset), wheelRadius, wheelThickness, wheelColor, 20, false, false);
        // Right Wheel - hasBlackTread = true
        AddCylinder(vertices, indices, glm::vec3(x, wheelY, -zOffset), wheelRadius, wheelThickness, wheelColor, 20, false, false);
    }

    Mesh *locomotive = new Mesh(name);
    locomotive->InitFromData(vertices, indices);
    return locomotive;
}

Mesh *object3D::CreateWagon(
    const std::string &name,
    glm::vec3 bodyColor,
    glm::vec3 wheelColor,
    glm::vec3 supportColor)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 baseColor(1.0f, 0.8f, 0.0f); // Yellow

    // 1. Base (Yellow)
    // Coordinates: Y from 0.30 to 0.45. Center Y is 0.375
    AddBox(vertices, indices, glm::vec3(-1.0f, 0.30f, -0.4f), glm::vec3(1.0f, 0.45f, 0.4f), baseColor, true);

    // 2. Body (Green)
    AddBox(vertices, indices, glm::vec3(-1.0f, 0.45f, -0.4f), glm::vec3(1.0f, 1.15f, 0.4f), bodyColor, true);

    // 3. Wheels (Red) - 4 CYLINDERS
    float wheelY = 0.15f;
    float wheelRadius = 0.15f;
    float wheelThickness = 0.12f;
    float zOffset = 0.3f;
    float xOffset = 0.7f;

    AddCylinder(vertices, indices, glm::vec3(xOffset, wheelY, zOffset), wheelRadius, wheelThickness, wheelColor, 20, false, false);
    AddCylinder(vertices, indices, glm::vec3(xOffset, wheelY, -zOffset), wheelRadius, wheelThickness, wheelColor, 20, false, false);
    AddCylinder(vertices, indices, glm::vec3(-xOffset, wheelY, zOffset), wheelRadius, wheelThickness, wheelColor, 20, false, false);
    AddCylinder(vertices, indices, glm::vec3(-xOffset, wheelY, -zOffset), wheelRadius, wheelThickness, wheelColor, 20, false, false);

    // 4. THICK RING (Gray with Black Gradient)
    {
        glm::vec3 ringColor(0.5f, 0.5f, 0.5f); // Gray (Interior)
        glm::vec3 black(0.0f, 0.0f, 0.0f);     // Black (Exterior)

        // Ring parameters
        glm::vec3 center(1.05f, 0.375f, 0.0f); // Positioning
        float outerRadius = 0.05f;
        float innerRadius = 0.03f;
        float thickness = 0.04f;
        int segments = 32;

        float halfThick = thickness / 2.0f;
        unsigned int ringStart = vertices.size();

        for (int i = 0; i < segments; i++)
        {
            float angle = 2.0f * M_PI * i / segments;
            float cosA = cos(angle);
            float sinA = sin(angle);

            // 0: Top-Inner -> Color: GRAY (Center)
            vertices.push_back(VertexFormat(
                glm::vec3(center.x + innerRadius * cosA, center.y + halfThick, center.z + innerRadius * sinA),
                ringColor));

            // 1: Top-Outer -> Color: BLACK (Edge)
            vertices.push_back(VertexFormat(
                glm::vec3(center.x + outerRadius * cosA, center.y + halfThick, center.z + outerRadius * sinA),
                black));

            // 2: Bottom-Inner -> Color: GRAY (Center)
            vertices.push_back(VertexFormat(
                glm::vec3(center.x + innerRadius * cosA, center.y - halfThick, center.z + innerRadius * sinA),
                ringColor));

            // 3: Bottom-Outer -> Color: BLACK (Edge)
            vertices.push_back(VertexFormat(
                glm::vec3(center.x + outerRadius * cosA, center.y - halfThick, center.z + outerRadius * sinA),
                black));

            // Indices
            int current = i * 4;
            int next = ((i + 1) % segments) * 4;

            unsigned int c0 = ringStart + current + 0;
            unsigned int c1 = ringStart + current + 1;
            unsigned int c2 = ringStart + current + 2;
            unsigned int c3 = ringStart + current + 3;

            unsigned int n0 = ringStart + next + 0;
            unsigned int n1 = ringStart + next + 1;
            unsigned int n2 = ringStart + next + 2;
            unsigned int n3 = ringStart + next + 3;

            // Top Face
            indices.push_back(c0);
            indices.push_back(c1);
            indices.push_back(n1);
            indices.push_back(c0);
            indices.push_back(n1);
            indices.push_back(n0);

            // Bottom Face
            indices.push_back(c2);
            indices.push_back(n3);
            indices.push_back(c3);
            indices.push_back(c2);
            indices.push_back(n2);
            indices.push_back(n3);

            // Outer Wall (Completely Black)
            indices.push_back(c1);
            indices.push_back(c3);
            indices.push_back(n3);
            indices.push_back(c1);
            indices.push_back(n3);
            indices.push_back(n1);

            // Inner Wall (Completely Gray)
            indices.push_back(c0);
            indices.push_back(n2);
            indices.push_back(c2);
            indices.push_back(c0);
            indices.push_back(n0);
            indices.push_back(n2);
        }
    }

    Mesh *wagon = new Mesh(name);
    wagon->InitFromData(vertices, indices);
    return wagon;
}

Mesh *object3D::CreateBasicTrainStation(
    const std::string &name,
    glm::vec3 platformColor,
    glm::vec3 wallColor,
    glm::vec3 doorColor,
    glm::vec3 symbolColor,
    glm::vec3 roofColor,
    const std::string &symbolShape)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // 1. Platform (small)
    AddBox(vertices, indices, glm::vec3(-1.5f, 0.0f, -1.5f), glm::vec3(1.5f, 0.2f, 1.5f), platformColor, true);

    // 2. House (Simple cube)
    AddBox(vertices, indices, glm::vec3(-1.0f, 0.2f, -1.0f), glm::vec3(1.0f, 1.5f, 1.0f), wallColor, true);

    // 3. Roof (Pyramid with outline)
    AddPyramid(vertices, indices, glm::vec3(0.0f, 1.5f, 0.0f), 2.4f, 1.0f, roofColor, true);

    // 4. Door (Stuck to house)
    AddBox(vertices, indices, glm::vec3(-0.3f, 0.2f, 1.0f), glm::vec3(0.3f, 1.2f, 1.05f), doorColor, true);

    // 5. Floating Symbol (Above roof)
    // The roof ends at y=2.5. We put the symbol starting from y=3.05
    if (symbolShape == "cube")
    {
        AddBox(vertices, indices, glm::vec3(-0.5f, 3.0f, -0.5f), glm::vec3(0.5f, 4.0f, 0.5f), symbolColor, true);
    }
    else if (symbolShape == "pyramid")
    {
        AddPyramid(vertices, indices, glm::vec3(0.0f, 2.8f, 0.0f), 1.0f, 1.0f, symbolColor, true);
    }
    else if (symbolShape == "sphere")
    {
        AddSphere(vertices, indices, glm::vec3(0.0f, 3.05f, 0.0f), 0.5f, symbolColor, 16, 16, true);
    }

    Mesh *station = new Mesh(name);
    station->InitFromData(vertices, indices);
    return station;
}

Mesh *object3D::CreateMainTrainStation(
    const std::string &name,
    glm::vec3 buildingColor,
    glm::vec3 roofColor,
    glm::vec3 platformColor)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // 1. Platform (identical to BasicTrainStation)
    AddBox(vertices, indices, glm::vec3(-1.5f, 0.0f, -1.5f), glm::vec3(1.5f, 0.2f, 1.5f), platformColor, true);

    // 2. Main Building (same dimensions as house in BasicTrainStation)
    AddBox(vertices, indices, glm::vec3(-1.0f, 0.2f, -1.0f), glm::vec3(1.0f, 1.5f, 1.0f), buildingColor, true);

    // 3. Roof (box instead of pyramid, but with outline)
    AddBox(vertices, indices, glm::vec3(-1.2f, 1.5f, -1.2f), glm::vec3(1.2f, 1.8f, 1.2f), roofColor, true);

    // 4. Chimney (On roof, with outline)
    glm::vec3 chimneyColor(0.4f, 0.4f, 0.4f);
    AddBox(vertices, indices, glm::vec3(0.5f, 1.8f, 0.3f), glm::vec3(0.8f, 2.5f, 0.6f), chimneyColor, true);

    // 5. Door (Stuck to house, with outline)
    glm::vec3 doorColor(0.4f, 0.2f, 0.1f);
    AddBox(vertices, indices, glm::vec3(-0.3f, 0.2f, 1.0f), glm::vec3(0.3f, 1.2f, 1.05f), doorColor, true);

    Mesh *station = new Mesh(name);
    station->InitFromData(vertices, indices);
    return station;
}

Mesh *object3D::CreateNormalRail(
    const std::string &name,
    glm::vec3 railColor,
    float length)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;
    // Wagon/locomotive length: 2.0 units (from -1.0 to 1.0)
    // Adjust length to be 2.0 to match perfectly
    length = 2.0f;

    // Wheel position: zOffset = 0.3
    // Wheel width: wheelThickness = 0.12, so it occupies from -0.06 to +0.06 relative to center
    // Wheel center at z = +/-0.3
    // Left wheel: from 0.3 - 0.06 = 0.24 to 0.3 + 0.06 = 0.36
    // Right wheel: from -0.36 to -0.24

    float railWidth = 0.05f;     // Rail width
    float railHeight = 0.08f;    // Rail height (slightly higher)
    float leftRailPos = 0.30f;   // Left rail position (aligned with wheel center)
    float rightRailPos = -0.30f; // Right rail position (aligned with wheel center)

    // Left Rail (aligned with left wheel)
    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, leftRailPos - railWidth),
           glm::vec3(length / 2, railHeight, leftRailPos + railWidth),
           railColor);

    // Right Rail (aligned with right wheel)
    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, rightRailPos - railWidth),
           glm::vec3(length / 2, railHeight, rightRailPos + railWidth),
           railColor);

    // Sleepers - placed at intervals
    glm::vec3 sleeperColor(0.5f, 0.3f, 0.1f);
    int numSleepers = 5; // Fixed number for length of 2.0
    float step = length / (numSleepers + 1);

    for (int i = 1; i <= numSleepers; i++)
    {
        float x = -length / 2 + i * step;
        AddBox(vertices, indices,
               glm::vec3(x - 0.05f, 0.0f, rightRailPos - railWidth - 0.1f),
               glm::vec3(x + 0.05f, railHeight * 0.5f, leftRailPos + railWidth + 0.1f),
               sleeperColor);
    }

    Mesh *rail = new Mesh(name);
    rail->InitFromData(vertices, indices);
    return rail;
}
Mesh *object3D::CreateTunnelRail(
    const std::string &name,
    glm::vec3 railColor,
    glm::vec3 tunnelColor,
    float length)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;
    // Adjusted length for wagon/locomotive
    length = 2.0f;

    // Wheel position: zOffset = 0.3
    float railWidth = 0.05f;
    float railHeight = 0.08f; // slightly higher
    float leftRailPos = 0.30f;
    float rightRailPos = -0.30f;

    // 1. Rails (aligned with wheels)
    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, leftRailPos - railWidth),
           glm::vec3(length / 2, railHeight, leftRailPos + railWidth),
           railColor);

    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, rightRailPos - railWidth),
           glm::vec3(length / 2, railHeight, rightRailPos + railWidth),
           railColor);

    // 2. Tunnel - sufficiently high and wide for locomotive
    // Locomotive: max height ~1.45 (base at 0.45 + cabin up to 1.15 + engine at 0.85)
    // Width: +/-0.4 for body
    float tunnelHeight = 2.0f; // Sufficiently high
    float tunnelWidth = 1.2f;  // Sufficiently wide
    float wallThickness = 0.3f;

    // Left Wall
    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, -tunnelWidth),
           glm::vec3(length / 2, tunnelHeight, -tunnelWidth + wallThickness),
           tunnelColor);

    // Right Wall
    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, tunnelWidth - wallThickness),
           glm::vec3(length / 2, tunnelHeight, tunnelWidth),
           tunnelColor);

    // Ceiling
    AddBox(vertices, indices,
           glm::vec3(-length / 2, tunnelHeight - wallThickness, -tunnelWidth),
           glm::vec3(length / 2, tunnelHeight, tunnelWidth),
           tunnelColor);

    // 3. White stripes (on ceiling, edges, length)
    glm::vec3 stripeColor(1.0f, 1.0f, 1.0f);
    float stripeWidth = 0.08f;
    float stripeThickness = 0.02f;

    // Top-Left Stripe
    AddBox(vertices, indices,
           glm::vec3(-length / 2, tunnelHeight + 0.01f, -tunnelWidth + wallThickness),
           glm::vec3(length / 2, tunnelHeight + 0.01f + stripeThickness, -tunnelWidth + wallThickness + stripeWidth),
           stripeColor);

    // Top-Right Stripe
    AddBox(vertices, indices,
           glm::vec3(-length / 2, tunnelHeight + 0.01f, tunnelWidth - wallThickness - stripeWidth),
           glm::vec3(length / 2, tunnelHeight + 0.01f + stripeThickness, tunnelWidth - wallThickness),
           stripeColor);

    Mesh *rail = new Mesh(name);
    rail->InitFromData(vertices, indices);
    return rail;
}
Mesh *object3D::CreateCube(
    const std::string &name,
    glm::vec3 center,
    float size,
    glm::vec3 color,
    bool hasBlackBorder)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float halfSize = size / 2.0f;
    glm::vec3 minPoint = center - glm::vec3(halfSize);
    glm::vec3 maxPoint = center + glm::vec3(halfSize);

    AddBox(vertices, indices, minPoint, maxPoint, color, hasBlackBorder);

    Mesh *cube = new Mesh(name);
    cube->InitFromData(vertices, indices);
    return cube;
}

Mesh *object3D::CreatePyramidMesh(
    const std::string &name,
    glm::vec3 baseCenter,
    float baseSize,
    float height,
    glm::vec3 color,
    bool hasBlackBorder)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    AddPyramid(vertices, indices, baseCenter, baseSize, height, color, hasBlackBorder);

    Mesh *pyramid = new Mesh(name);
    pyramid->InitFromData(vertices, indices);
    return pyramid;
}

Mesh *object3D::CreateSphere(
    const std::string &name,
    glm::vec3 center,
    float radius,
    glm::vec3 color,
    bool hasBlackBorder)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    AddSphere(vertices, indices, center, radius, color, 16, 16, hasBlackBorder);

    Mesh *sphere = new Mesh(name);
    sphere->InitFromData(vertices, indices);
    return sphere;
}

Mesh *object3D::CreateBridgeRail(
    const std::string &name,
    glm::vec3 railColor,
    glm::vec3 bridgeColor,
    float length)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;
    // Adjusted length for wagon/locomotive
    length = 2.0f;

    // Wheel position: zOffset = 0.3
    float railWidth = 0.05f;
    float railHeight = 0.08f; // slightly higher
    float leftRailPos = 0.30f;
    float rightRailPos = -0.30f;

    // Bridge: slightly raised and thinner to not go under the map
    float bridgeTopPlatform = 0.02f;
    float bridgeThickness = 0.06f; // reduced thickness

    // 1. Rails (raised above the bridge)
    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, leftRailPos - railWidth),
           glm::vec3(length / 2, 0.0f + railHeight, leftRailPos + railWidth),
           railColor);

    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, rightRailPos - railWidth),
           glm::vec3(length / 2, 0.0f + railHeight, rightRailPos + railWidth),
           railColor);

    // 2. Bridge (Platform under rails, sufficiently wide)
    float bridgeWidth = 1.0f; // Sufficient width for visual stability
    AddBox(vertices, indices,
           glm::vec3(-length / 2, bridgeTopPlatform - bridgeThickness, -bridgeWidth),
           glm::vec3(length / 2, bridgeTopPlatform, bridgeWidth),
           bridgeColor);

    // 3. Transverse white stripes ("multiple times per segment")
    glm::vec3 stripeColor(1.0f, 1.0f, 1.0f);
    int numStripes = 6;
    float spacing = length / (numStripes + 1);

    for (int i = 1; i <= numStripes; i++)
    {
        float xPos = -length / 2 + i * spacing;
        // Transverse stripe on bridge
        AddBox(vertices, indices,
               glm::vec3(xPos - 0.03f, bridgeTopPlatform + 0.01f, -bridgeWidth * 0.9f),
               glm::vec3(xPos + 0.03f, bridgeTopPlatform + 0.02f, bridgeWidth * 0.9f),
               stripeColor);
    }

    Mesh *rail = new Mesh(name);
    rail->InitFromData(vertices, indices);
    return rail;
}

Mesh *object3D::CreateNormalRailIntersection(
    const std::string &name,
    glm::vec3 railColor,
    bool hasNorth, bool hasSouth, bool hasEast, bool hasWest)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float length = 2.0f;
    float halfLen = length / 2.0f;
    float railWidth = 0.05f;
    float railHeight = 0.08f;
    float railPos = 0.30f; // distance from center for both rail pairs

    // Sleepers - in the center, always present
    glm::vec3 sleeperColor(0.5f, 0.3f, 0.1f);

    // Add sleepers based on which directions have rails
    if (hasEast || hasWest)
    {
        // Horizontal sleepers for E-W direction
        int numSleepers = 3;
        float step = length / (numSleepers + 1);
        for (int i = 1; i <= numSleepers; i++)
        {
            float x = -halfLen + i * step;
            if (x > -halfLen * 0.4f && x < halfLen * 0.4f) // only in center area
            {
                AddBox(vertices, indices,
                       glm::vec3(x - 0.05f, 0.0f, -railPos - railWidth - 0.1f),
                       glm::vec3(x + 0.05f, railHeight * 0.5f, railPos + railWidth + 0.1f),
                       sleeperColor);
            }
        }
    }

    if (hasNorth || hasSouth)
    {
        // Vertical sleepers for N-S direction
        int numSleepers = 3;
        float step = length / (numSleepers + 1);
        for (int i = 1; i <= numSleepers; i++)
        {
            float z = -halfLen + i * step;
            if (z > -halfLen * 0.4f && z < halfLen * 0.4f) // only in center area
            {
                AddBox(vertices, indices,
                       glm::vec3(-railPos - railWidth - 0.1f, 0.0f, z - 0.05f),
                       glm::vec3(railPos + railWidth + 0.1f, railHeight * 0.5f, z + 0.05f),
                       sleeperColor);
            }
        }
    }

    // For East-West direction: rails run along X axis at Z = +/-railPos
    // For North-South direction: rails run along Z axis at X = +/-railPos
    // Rails must extend to the edge of the mesh (halfLen) to connect with adjacent rail segments

    // East-West rails (Z = +/-railPos, extending in X direction)
    if (hasEast && hasWest)
    {
        // Full E-W rails from edge to edge
        AddBox(vertices, indices,
               glm::vec3(-halfLen, 0.0f, railPos - railWidth),
               glm::vec3(halfLen, railHeight, railPos + railWidth),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(-halfLen, 0.0f, -railPos - railWidth),
               glm::vec3(halfLen, railHeight, -railPos + railWidth),
               railColor);
    }
    else if (hasEast)
    {
        // Only East - rails from center area to East edge
        // Start from -railPos to connect with N-S rails if they exist
        float startX = (hasNorth || hasSouth) ? -railPos : 0.0f;
        AddBox(vertices, indices,
               glm::vec3(startX, 0.0f, railPos - railWidth),
               glm::vec3(halfLen, railHeight, railPos + railWidth),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(startX, 0.0f, -railPos - railWidth),
               glm::vec3(halfLen, railHeight, -railPos + railWidth),
               railColor);
    }
    else if (hasWest)
    {
        // Only West - rails from West edge to center area
        float endX = (hasNorth || hasSouth) ? railPos : 0.0f;
        AddBox(vertices, indices,
               glm::vec3(-halfLen, 0.0f, railPos - railWidth),
               glm::vec3(endX, railHeight, railPos + railWidth),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(-halfLen, 0.0f, -railPos - railWidth),
               glm::vec3(endX, railHeight, -railPos + railWidth),
               railColor);
    }

    // North-South rails (X = +/-railPos, extending in Z direction)
    if (hasNorth && hasSouth)
    {
        // Full N-S rails from edge to edge
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, -halfLen),
               glm::vec3(railPos + railWidth, railHeight, halfLen),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, -halfLen),
               glm::vec3(-railPos + railWidth, railHeight, halfLen),
               railColor);
    }
    else if (hasNorth)
    {
        // Only North - rails from center area to North edge
        float startZ = (hasEast || hasWest) ? -railPos : 0.0f;
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, startZ),
               glm::vec3(railPos + railWidth, railHeight, halfLen),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, startZ),
               glm::vec3(-railPos + railWidth, railHeight, halfLen),
               railColor);
    }
    else if (hasSouth)
    {
        // Only South - rails from South edge to center area
        float endZ = (hasEast || hasWest) ? railPos : 0.0f;
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, -halfLen),
               glm::vec3(railPos + railWidth, railHeight, endZ),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, -halfLen),
               glm::vec3(-railPos + railWidth, railHeight, endZ),
               railColor);
    }

    // Corner connectors - short perpendicular segments to connect turning rails
    // These allow the train wheels to transition between directions
    float cornerSize = railPos + railWidth;

    // If can go East and North - need connector at +X,+Z corner
    if (hasEast && hasNorth)
    {
        // Horizontal segment at +Z connecting E-W rail to N-S rail
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, railPos - railWidth),
               glm::vec3(railPos + railWidth, railHeight, railPos + railWidth),
               railColor);
    }
    if (hasEast && hasSouth)
    {
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, -railPos - railWidth),
               glm::vec3(railPos + railWidth, railHeight, -railPos + railWidth),
               railColor);
    }
    if (hasWest && hasNorth)
    {
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, railPos - railWidth),
               glm::vec3(-railPos + railWidth, railHeight, railPos + railWidth),
               railColor);
    }
    if (hasWest && hasSouth)
    {
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, -railPos - railWidth),
               glm::vec3(-railPos + railWidth, railHeight, -railPos + railWidth),
               railColor);
    }

    // Cross bars for 3+ way intersections
    // These connect the parallel rails across the intersection
    int numDirections = (hasEast ? 1 : 0) + (hasWest ? 1 : 0) + (hasNorth ? 1 : 0) + (hasSouth ? 1 : 0);

    if (numDirections >= 3)
    {
        // Horizontal bar at Z = +railPos (north side) - connects left N-S rail to right N-S rail
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, railPos - railWidth),
               glm::vec3(railPos + railWidth, railHeight, railPos + railWidth),
               railColor);

        // Horizontal bar at Z = -railPos (south side) - connects left N-S rail to right N-S rail
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, -railPos - railWidth),
               glm::vec3(railPos + railWidth, railHeight, -railPos + railWidth),
               railColor);

        // Vertical bar at X = +railPos (east side) - connects front E-W rail to back E-W rail
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, -railPos - railWidth),
               glm::vec3(railPos + railWidth, railHeight, railPos + railWidth),
               railColor);

        // Vertical bar at X = -railPos (west side) - connects front E-W rail to back E-W rail
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, -railPos - railWidth),
               glm::vec3(-railPos + railWidth, railHeight, railPos + railWidth),
               railColor);
    }

    Mesh *rail = new Mesh(name);
    rail->InitFromData(vertices, indices);
    return rail;
}

Mesh *object3D::CreateTunnelRailIntersection(
    const std::string &name,
    glm::vec3 railColor,
    glm::vec3 tunnelColor,
    bool hasNorth, bool hasSouth, bool hasEast, bool hasWest)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float length = 2.0f;
    float halfLen = length / 2.0f;
    float railWidth = 0.05f;
    float railHeight = 0.08f;
    float railPos = 0.30f;

    // RAIL LOGIC (same as normal intersection)
    // East-West rails
    if (hasEast && hasWest)
    {
        AddBox(vertices, indices,
               glm::vec3(-halfLen, 0.0f, railPos - railWidth),
               glm::vec3(halfLen, railHeight, railPos + railWidth),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(-halfLen, 0.0f, -railPos - railWidth),
               glm::vec3(halfLen, railHeight, -railPos + railWidth),
               railColor);
    }
    else if (hasEast)
    {
        float startX = (hasNorth || hasSouth) ? -railPos : 0.0f;
        AddBox(vertices, indices,
               glm::vec3(startX, 0.0f, railPos - railWidth),
               glm::vec3(halfLen, railHeight, railPos + railWidth),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(startX, 0.0f, -railPos - railWidth),
               glm::vec3(halfLen, railHeight, -railPos + railWidth),
               railColor);
    }
    else if (hasWest)
    {
        float endX = (hasNorth || hasSouth) ? railPos : 0.0f;
        AddBox(vertices, indices,
               glm::vec3(-halfLen, 0.0f, railPos - railWidth),
               glm::vec3(endX, railHeight, railPos + railWidth),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(-halfLen, 0.0f, -railPos - railWidth),
               glm::vec3(endX, railHeight, -railPos + railWidth),
               railColor);
    }

    // North-South rails
    if (hasNorth && hasSouth)
    {
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, -halfLen),
               glm::vec3(railPos + railWidth, railHeight, halfLen),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, -halfLen),
               glm::vec3(-railPos + railWidth, railHeight, halfLen),
               railColor);
    }
    else if (hasNorth)
    {
        float startZ = (hasEast || hasWest) ? -railPos : 0.0f;
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, startZ),
               glm::vec3(railPos + railWidth, railHeight, halfLen),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, startZ),
               glm::vec3(-railPos + railWidth, railHeight, halfLen),
               railColor);
    }
    else if (hasSouth)
    {
        float endZ = (hasEast || hasWest) ? railPos : 0.0f;
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, -halfLen),
               glm::vec3(railPos + railWidth, railHeight, endZ),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, -halfLen),
               glm::vec3(-railPos + railWidth, railHeight, endZ),
               railColor);
    }

    // Corner connectors
    if (hasEast && hasNorth)
    {
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, railPos - railWidth),
               glm::vec3(railPos + railWidth, railHeight, railPos + railWidth),
               railColor);
    }
    if (hasEast && hasSouth)
    {
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, -railPos - railWidth),
               glm::vec3(railPos + railWidth, railHeight, -railPos + railWidth),
               railColor);
    }
    if (hasWest && hasNorth)
    {
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, railPos - railWidth),
               glm::vec3(-railPos + railWidth, railHeight, railPos + railWidth),
               railColor);
    }
    if (hasWest && hasSouth)
    {
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, -railPos - railWidth),
               glm::vec3(-railPos + railWidth, railHeight, -railPos + railWidth),
               railColor);
    }

    // TUNNEL STRUCTURE
    // Use same dimensions as regular tunnel for alignment
    float tunnelHeight = 2.0f;
    float tunnelWidth = 1.2f;
    float wallThickness = 0.3f;

    // Walls - only where there are NO connections, positioned to align with regular tunnel
    // All walls use tunnelWidth as the extent to make the intersection square
    // South wall (-Z side)
    if (!hasSouth)
    {
        AddBox(vertices, indices,
               glm::vec3(-tunnelWidth, 0.0f, -tunnelWidth),
               glm::vec3(tunnelWidth, tunnelHeight, -tunnelWidth + wallThickness),
               tunnelColor);
    }

    // North wall (+Z side)
    if (!hasNorth)
    {
        AddBox(vertices, indices,
               glm::vec3(-tunnelWidth, 0.0f, tunnelWidth - wallThickness),
               glm::vec3(tunnelWidth, tunnelHeight, tunnelWidth),
               tunnelColor);
    }

    // West wall (-X side)
    if (!hasWest)
    {
        AddBox(vertices, indices,
               glm::vec3(-tunnelWidth, 0.0f, -tunnelWidth),
               glm::vec3(-tunnelWidth + wallThickness, tunnelHeight, tunnelWidth),
               tunnelColor);
    }

    // East wall (+X side)
    if (!hasEast)
    {
        AddBox(vertices, indices,
               glm::vec3(tunnelWidth - wallThickness, 0.0f, -tunnelWidth),
               glm::vec3(tunnelWidth, tunnelHeight, tunnelWidth),
               tunnelColor);
    }

    // Ceiling - covers the full intersection area (square)
    AddBox(vertices, indices,
           glm::vec3(-tunnelWidth, tunnelHeight - wallThickness, -tunnelWidth),
           glm::vec3(tunnelWidth, tunnelHeight, tunnelWidth),
           tunnelColor);

    // White stripes on ceiling
    glm::vec3 stripeColor(1.0f, 1.0f, 1.0f);
    float stripeWidth = 0.08f;
    float stripeThickness = 0.02f;

    // Longitudinal stripes (along X axis) - only if that side has a connection (no wall)
    if (hasSouth)
    {
        AddBox(vertices, indices,
               glm::vec3(-tunnelWidth, tunnelHeight + 0.01f, -tunnelWidth + wallThickness),
               glm::vec3(tunnelWidth, tunnelHeight + 0.01f + stripeThickness, -tunnelWidth + wallThickness + stripeWidth),
               stripeColor);
    }
    if (hasNorth)
    {
        AddBox(vertices, indices,
               glm::vec3(-tunnelWidth, tunnelHeight + 0.01f, tunnelWidth - wallThickness - stripeWidth),
               glm::vec3(tunnelWidth, tunnelHeight + 0.01f + stripeThickness, tunnelWidth - wallThickness),
               stripeColor);
    }

    // Perpendicular stripes above walls (where there's no connection)
    // North wall stripe (horizontal along X, at +Z edge)
    if (!hasNorth)
    {
        AddBox(vertices, indices,
               glm::vec3(-tunnelWidth, tunnelHeight + 0.01f, tunnelWidth - wallThickness - stripeWidth),
               glm::vec3(tunnelWidth, tunnelHeight + 0.01f + stripeThickness, tunnelWidth - wallThickness),
               stripeColor);
    }
    // South wall stripe (horizontal along X, at -Z edge)
    if (!hasSouth)
    {
        AddBox(vertices, indices,
               glm::vec3(-tunnelWidth, tunnelHeight + 0.01f, -tunnelWidth + wallThickness),
               glm::vec3(tunnelWidth, tunnelHeight + 0.01f + stripeThickness, -tunnelWidth + wallThickness + stripeWidth),
               stripeColor);
    }
    // West wall stripe (vertical along Z, at -X edge)
    if (!hasWest)
    {
        AddBox(vertices, indices,
               glm::vec3(-tunnelWidth + wallThickness, tunnelHeight + 0.01f, -tunnelWidth),
               glm::vec3(-tunnelWidth + wallThickness + stripeWidth, tunnelHeight + 0.01f + stripeThickness, tunnelWidth),
               stripeColor);
    }
    // East wall stripe (vertical along Z, at +X edge)
    if (!hasEast)
    {
        AddBox(vertices, indices,
               glm::vec3(tunnelWidth - wallThickness - stripeWidth, tunnelHeight + 0.01f, -tunnelWidth),
               glm::vec3(tunnelWidth - wallThickness, tunnelHeight + 0.01f + stripeThickness, tunnelWidth),
               stripeColor);
    }

    Mesh *rail = new Mesh(name);
    rail->InitFromData(vertices, indices);
    return rail;
}

Mesh *object3D::CreateBridgeRailIntersection(
    const std::string &name,
    glm::vec3 railColor,
    glm::vec3 bridgeColor,
    bool hasNorth, bool hasSouth, bool hasEast, bool hasWest)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float length = 2.0f;
    float halfLen = length / 2.0f;
    float railWidth = 0.05f;
    float railHeight = 0.08f;
    float railPos = 0.30f;

    // Bridge platform (always present)
    float bridgeTopPlatform = 0.02f;
    float bridgeThickness = 0.06f;
    float bridgeWidth = 1.0f;

    AddBox(vertices, indices,
           glm::vec3(-halfLen, bridgeTopPlatform - bridgeThickness, -bridgeWidth),
           glm::vec3(halfLen, bridgeTopPlatform, bridgeWidth),
           bridgeColor);

    // RAIL LOGIC (same as normal intersection)
    // East-West rails
    if (hasEast && hasWest)
    {
        AddBox(vertices, indices,
               glm::vec3(-halfLen, 0.0f, railPos - railWidth),
               glm::vec3(halfLen, railHeight, railPos + railWidth),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(-halfLen, 0.0f, -railPos - railWidth),
               glm::vec3(halfLen, railHeight, -railPos + railWidth),
               railColor);
    }
    else if (hasEast)
    {
        float startX = (hasNorth || hasSouth) ? -railPos : 0.0f;
        AddBox(vertices, indices,
               glm::vec3(startX, 0.0f, railPos - railWidth),
               glm::vec3(halfLen, railHeight, railPos + railWidth),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(startX, 0.0f, -railPos - railWidth),
               glm::vec3(halfLen, railHeight, -railPos + railWidth),
               railColor);
    }
    else if (hasWest)
    {
        float endX = (hasNorth || hasSouth) ? railPos : 0.0f;
        AddBox(vertices, indices,
               glm::vec3(-halfLen, 0.0f, railPos - railWidth),
               glm::vec3(endX, railHeight, railPos + railWidth),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(-halfLen, 0.0f, -railPos - railWidth),
               glm::vec3(endX, railHeight, -railPos + railWidth),
               railColor);
    }

    // North-South rails
    if (hasNorth && hasSouth)
    {
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, -halfLen),
               glm::vec3(railPos + railWidth, railHeight, halfLen),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, -halfLen),
               glm::vec3(-railPos + railWidth, railHeight, halfLen),
               railColor);
    }
    else if (hasNorth)
    {
        float startZ = (hasEast || hasWest) ? -railPos : 0.0f;
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, startZ),
               glm::vec3(railPos + railWidth, railHeight, halfLen),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, startZ),
               glm::vec3(-railPos + railWidth, railHeight, halfLen),
               railColor);
    }
    else if (hasSouth)
    {
        float endZ = (hasEast || hasWest) ? railPos : 0.0f;
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, -halfLen),
               glm::vec3(railPos + railWidth, railHeight, endZ),
               railColor);
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, -halfLen),
               glm::vec3(-railPos + railWidth, railHeight, endZ),
               railColor);
    }

    // Corner connectors
    if (hasEast && hasNorth)
    {
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, railPos - railWidth),
               glm::vec3(railPos + railWidth, railHeight, railPos + railWidth),
               railColor);
    }
    if (hasEast && hasSouth)
    {
        AddBox(vertices, indices,
               glm::vec3(railPos - railWidth, 0.0f, -railPos - railWidth),
               glm::vec3(railPos + railWidth, railHeight, -railPos + railWidth),
               railColor);
    }
    if (hasWest && hasNorth)
    {
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, railPos - railWidth),
               glm::vec3(-railPos + railWidth, railHeight, railPos + railWidth),
               railColor);
    }
    if (hasWest && hasSouth)
    {
        AddBox(vertices, indices,
               glm::vec3(-railPos - railWidth, 0.0f, -railPos - railWidth),
               glm::vec3(-railPos + railWidth, railHeight, -railPos + railWidth),
               railColor);
    }

    // White stripes on bridge
    glm::vec3 stripeColor(1.0f, 1.0f, 1.0f);
    int numStripes = 6;
    float spacing = length / (numStripes + 1);

    for (int i = 1; i <= numStripes; i++)
    {
        float xPos = -halfLen + i * spacing;
        AddBox(vertices, indices,
               glm::vec3(xPos - 0.03f, bridgeTopPlatform + 0.01f, -bridgeWidth * 0.9f),
               glm::vec3(xPos + 0.03f, bridgeTopPlatform + 0.02f, bridgeWidth * 0.9f),
               stripeColor);
    }

    Mesh *rail = new Mesh(name);
    rail->InitFromData(vertices, indices);
    return rail;
}