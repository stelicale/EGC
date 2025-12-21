#include "object3D.hpp"

#include <vector>
#include <cmath>

#include "core/engine.h"
#include "utils/gl_utils.h"

// 2D

// 1. Funcție pentru generarea unui CERC (Turcuaz - pentru stații)
Mesh* object3D::CreateCircle(const std::string& name, glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Centrul cercului
    vertices.emplace_back(glm::vec3(0, 0, 0), color);
    
    // Generăm puncte pe circumferință
    int numSegments = 32;
    float radius = 1.0f;

    for (int i = 0; i <= numSegments; i++) {
        float theta = 2.0f * 3.14159f * float(i) / float(numSegments);
        float x = radius * cosf(theta);
        float y = radius * sinf(theta); // Construim în planul XY

        vertices.emplace_back(glm::vec3(x, y, 0), color);
        
        // Creăm indicii pentru GL_TRIANGLE_FAN
        if (i < numSegments) {
            indices.push_back(0);        // Centru
            indices.push_back(i + 1);    // Punct curent
            indices.push_back(i + 2);    // Punct următor
        }
    }

    Mesh* circle = new Mesh(name);
    circle->InitFromData(vertices, indices);
    return circle;
}

// 2. Funcție pentru generarea unui TRIUNGHI (Verde - pentru pickup)
Mesh* object3D::CreateTriangle(const std::string& name, glm::vec3 color)
{
    std::vector<VertexFormat> vertices =
    {
        VertexFormat(glm::vec3(0, 0.5f, 0), color),      // Vârf sus
        VertexFormat(glm::vec3(-0.5f, -0.5f, 0), color), // Stânga jos
        VertexFormat(glm::vec3(0.5f, -0.5f, 0), color)   // Dreapta jos
    };

    std::vector<unsigned int> indices = { 0, 1, 2 };

    Mesh* triangle = new Mesh(name);
    triangle->InitFromData(vertices, indices);
    return triangle;
}

// 3. Funcție pentru generarea unei SĂGEȚI (Albă - pentru player/cameră)
Mesh* object3D::CreateArrow(const std::string& name, glm::vec3 color)
{
    std::vector<VertexFormat> vertices =
    {
        // Vârful săgeții
        VertexFormat(glm::vec3(0, 0.5f, 0), color),      
        VertexFormat(glm::vec3(-0.3f, 0.0f, 0), color),  
        VertexFormat(glm::vec3(0.3f, 0.0f, 0), color),   
        
        // Coada săgeții
        VertexFormat(glm::vec3(-0.1f, 0.0f, 0), color),  
        VertexFormat(glm::vec3(0.1f, 0.0f, 0), color),
        VertexFormat(glm::vec3(-0.1f, -0.5f, 0), color),
        VertexFormat(glm::vec3(0.1f, -0.5f, 0), color)
    };

    std::vector<unsigned int> indices = 
    { 
        0, 1, 2,        // Triunghi vârf
        3, 4, 6, 3, 6, 5 // Dreptunghi coadă
    };

    Mesh* arrow = new Mesh(name);
    arrow->InitFromData(vertices, indices);
    return arrow;
}

Mesh* object3D::CreateStar(const std::string& name, glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    vertices.emplace_back(glm::vec3(0, 0, 0), color); // Centru

    float rOuter = 1.0f;
    float rInner = 0.4f;
    int numPoints = 5;

    for (int i = 0; i <= numPoints * 2; i++) {
        // Alternăm razele (extern, intern, extern...)
        float radius = (i % 2 == 0) ? rOuter : rInner;
        // Unghiul curent: 360 grade / (2 * 5 puncte) = 36 grade per pas
        float currAngle = (i * 3.14159f) / numPoints; // PI / 5
        
        // Rotim cu PI/2 (90 grade) ca steaua să stea "dreaptă" (cu vârful în sus)
        float x = radius * cosf(currAngle + 1.5708f); 
        float y = radius * sinf(currAngle + 1.5708f);
        
        vertices.emplace_back(glm::vec3(x, y, 0), color);

        if (i < numPoints * 2) {
            indices.push_back(0);
            indices.push_back(i + 1);
            indices.push_back(i + 2);
        }
    }

    Mesh* star = new Mesh(name);
    star->InitFromData(vertices, indices);
    return star;
}

// 4. Funcție pentru generarea unui PĂTRAT (folosit pentru obstacole/tren)
Mesh *object3D::CreateSquare(const std::string& name, glm::vec3 color)
{
    std::vector<VertexFormat> vertices =
    {
        VertexFormat(glm::vec3(-0.5f, -0.5f, 0), color),
        VertexFormat(glm::vec3(0.5f, -0.5f, 0), color),
        VertexFormat(glm::vec3(0.5f, 0.5f, 0), color),
        VertexFormat(glm::vec3(-0.5f, 0.5f, 0), color)
    };

    std::vector<unsigned int> indices = { 0, 1, 2, 0, 2, 3 };

    Mesh* square = new Mesh(name);
    square->InitFromData(vertices, indices);
    return square;
}

// Helper intern pentru a adauga un cub (paralelipiped) in listele de varfuri si indici
// Aceasta functie reduce duplicarea codului
void AddBox(std::vector<VertexFormat> &vertices, std::vector<unsigned int> &indices,
            glm::vec3 minPoint, glm::vec3 maxPoint, glm::vec3 color, bool hasBlackBorder = false)
{
    // Definim cele 8 colțuri ale cutiei
    glm::vec3 p0 = glm::vec3(minPoint.x, minPoint.y, maxPoint.z); // Stanga-Jos-Fata
    glm::vec3 p1 = glm::vec3(maxPoint.x, minPoint.y, maxPoint.z); // Dreapta-Jos-Fata
    glm::vec3 p2 = glm::vec3(maxPoint.x, maxPoint.y, maxPoint.z); // Dreapta-Sus-Fata
    glm::vec3 p3 = glm::vec3(minPoint.x, maxPoint.y, maxPoint.z); // Stanga-Sus-Fata
    glm::vec3 p4 = glm::vec3(minPoint.x, minPoint.y, minPoint.z); // Stanga-Jos-Spate
    glm::vec3 p5 = glm::vec3(maxPoint.x, minPoint.y, minPoint.z); // Dreapta-Jos-Spate
    glm::vec3 p6 = glm::vec3(maxPoint.x, maxPoint.y, minPoint.z); // Dreapta-Sus-Spate
    glm::vec3 p7 = glm::vec3(minPoint.x, maxPoint.y, minPoint.z); // Stanga-Sus-Spate

    glm::vec3 black = glm::vec3(0.0f, 0.0f, 0.0f);
    float thickness = 0.02f; // Grosimea fixă a conturului (ajustează dacă e nevoie)

    auto CreateFace = [&](glm::vec3 A, glm::vec3 B, glm::vec3 C, glm::vec3 D, glm::vec3 faceColor)
    {
        unsigned int faceStart = vertices.size();

        if (hasBlackBorder)
        {
            // Calculăm direcțiile pentru a obține punctele interioare la distanță fixă
            // Pentru dreptunghiuri axate pe axe, direcțiile sunt simple vectori unitari.
            // A -> B este o latură, A -> D este cealaltă.

            glm::vec3 AB = glm::normalize(B - A);
            glm::vec3 AD = glm::normalize(D - A);
            glm::vec3 CB = glm::normalize(B - C);
            glm::vec3 CD = glm::normalize(D - C);

            // Calculăm colțurile interioare translatând cu 'thickness' pe ambele direcții
            glm::vec3 iA = A + (AB + AD) * thickness;
            glm::vec3 iB = B + (glm::normalize(A - B) + glm::normalize(C - B)) * thickness;
            glm::vec3 iC = C + (CB + CD) * thickness;
            glm::vec3 iD = D + (glm::normalize(A - D) + glm::normalize(C - D)) * thickness;

            // 1. MIEZUL COLORAT (4 vârfuri interioare)
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

            // 2. RAMA NEAGRĂ (8 vârfuri: 4 exterioare + 4 interioare duplicate ca negre)
            unsigned int frameStart = vertices.size();

            // Exterior (Negru)
            vertices.push_back(VertexFormat(A, black)); // 0
            vertices.push_back(VertexFormat(B, black)); // 1
            vertices.push_back(VertexFormat(C, black)); // 2
            vertices.push_back(VertexFormat(D, black)); // 3

            // Interior (Negru) - coincide spațial cu iA, iB...
            vertices.push_back(VertexFormat(iA, black)); // 4
            vertices.push_back(VertexFormat(iB, black)); // 5
            vertices.push_back(VertexFormat(iC, black)); // 6
            vertices.push_back(VertexFormat(iD, black)); // 7

            // Triunghiuri ramă
            // Jos
            indices.push_back(frameStart + 0);
            indices.push_back(frameStart + 1);
            indices.push_back(frameStart + 5);
            indices.push_back(frameStart + 0);
            indices.push_back(frameStart + 5);
            indices.push_back(frameStart + 4);
            // Dreapta
            indices.push_back(frameStart + 1);
            indices.push_back(frameStart + 2);
            indices.push_back(frameStart + 6);
            indices.push_back(frameStart + 1);
            indices.push_back(frameStart + 6);
            indices.push_back(frameStart + 5);
            // Sus
            indices.push_back(frameStart + 2);
            indices.push_back(frameStart + 3);
            indices.push_back(frameStart + 7);
            indices.push_back(frameStart + 2);
            indices.push_back(frameStart + 7);
            indices.push_back(frameStart + 6);
            // Stanga
            indices.push_back(frameStart + 3);
            indices.push_back(frameStart + 0);
            indices.push_back(frameStart + 4);
            indices.push_back(frameStart + 3);
            indices.push_back(frameStart + 4);
            indices.push_back(frameStart + 7);
        }
        else
        {
            // Standard (fără border)
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

    // Construim cele 6 fețe
    CreateFace(p0, p1, p2, p3, color); // Fata
    CreateFace(p5, p4, p7, p6, color); // Spate
    CreateFace(p4, p0, p3, p7, color); // Stanga
    CreateFace(p1, p5, p6, p2, color); // Dreapta
    CreateFace(p3, p2, p6, p7, color); // Sus
    CreateFace(p4, p5, p1, p0, color); // Jos
}

Mesh *object3D::CreateGround(
    const std::string &name,
    float length,
    glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float halfLength = length / 2.0f;
    // Il punem putin sub 0 (-0.01f) ca sa nu se suprapuna urat cu sinele care sunt la 0
    float yPos = -0.01f;

    // 4 varfuri formand un patrat mare in planul XZ
    vertices.push_back(VertexFormat(glm::vec3(-halfLength, yPos, -halfLength), color)); // 0
    vertices.push_back(VertexFormat(glm::vec3(halfLength, yPos, -halfLength), color));  // 1
    vertices.push_back(VertexFormat(glm::vec3(halfLength, yPos, halfLength), color));   // 2
    vertices.push_back(VertexFormat(glm::vec3(-halfLength, yPos, halfLength), color));  // 3

    // Doua triunghiuri
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
    float width, float height) // height aici reprezinta lungimea pe axa Z
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 waterColor(0.0f, 0.5f, 0.8f); // Albastru

    // Apa este un dreptunghi plat (foarte subțire) pe sol
    // Centrat în origine pe X și Z, la o înălțime y foarte mică sub șine
    float yPos = 0.0f;
    float thicknessY = 0.001f; // Foarte subțire, practic plat

    glm::vec3 minPoint(-width / 2.0f, yPos, -height / 2.0f);
    glm::vec3 maxPoint(width / 2.0f, yPos + thicknessY, height / 2.0f);

    // Folosim AddBox cu hasBlackBorder = true pentru a avea contur
    AddBox(vertices, indices, minPoint, maxPoint, waterColor, false);

    Mesh *water = new Mesh(name);
    water->InitFromData(vertices, indices);
    return water;
}

Mesh *object3D::CreateMountain(
    const std::string &name,
    float width, float height) // height aici reprezinta înălțimea vârfului pe Y
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 mountainColor(0.55f, 0.45f, 0.35f); // Maro

    // Muntele este un triunghi în planul XY
    // Centrat în origine pe X și Y, la o înălțime y foarte mică sub șine
    float yPos = 0.0f;
    float thicknessY = 0.001f; // Foarte subțire, practic plat

    glm::vec3 minPoint(-width / 2.0f, yPos, -height / 2.0f);
    glm::vec3 maxPoint(width / 2.0f, yPos + thicknessY, height / 2.0f);

    // Folosim AddBox cu hasBlackBorder = true pentru a avea contur
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

    // --- 1. PARTEA LATERALA (MANTAUA) ---
    unsigned int sideStart = vertices.size();

    // Lambda pentru a calcula poziția unui punct pe cerc
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
        // CAZ A: Cu bandă colorată - Cilindru cu benzi (negru-culoare-negru)
        float hStart = axisX ? -height / 2 : -height / 2;
        float hEnd = axisX ? height / 2 : height / 2;
        float hInnerStart = hStart + thickness;
        float hInnerEnd = hEnd - thickness;

        // Generăm inele de vârfuri
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

        // Indici (3 seturi de quad-uri)
        for (int i = 0; i < segments; i++)
        {
            int base = i * 6;
            int next = (i + 1) * 6;

            // 1. Banda Neagră de Start (0 -> 1)
            indices.push_back(sideStart + base + 0);
            indices.push_back(sideStart + next + 0);
            indices.push_back(sideStart + base + 1);
            indices.push_back(sideStart + base + 1);
            indices.push_back(sideStart + next + 0);
            indices.push_back(sideStart + next + 1);

            // 2. Banda Colorată din Mijloc (2 -> 3)
            indices.push_back(sideStart + base + 2);
            indices.push_back(sideStart + next + 2);
            indices.push_back(sideStart + base + 3);
            indices.push_back(sideStart + base + 3);
            indices.push_back(sideStart + next + 2);
            indices.push_back(sideStart + next + 3);

            // 3. Banda Neagră de Final (4 -> 5)
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
        // CAZ B: Fără bandă colorată - Cilindru lateral complet negru
        for (int i = 0; i <= segments; i++)
        {
            float angle = 2.0f * M_PI * i / segments;
            vertices.push_back(VertexFormat(GetPoint(axisX ? -height / 2 : -height / 2, angle), black));
            vertices.push_back(VertexFormat(GetPoint(axisX ? height / 2 : height / 2, angle), black));
        }
        // Indici (un singur set de quad-uri)
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

    // --- 2. CAPACELE (CAPS) ---
    auto CreateCap = [&](bool isFront)
    {
        float zPos = isFront ? height / 2 : -height / 2;
        float xOffset = axisX ? zPos : 0.0f;
        float zOffset = axisX ? 0.0f : zPos;

        // Capacele au mereu outline negru + cerc interior colorat
        float innerRadius = radius - thickness;
        if (innerRadius < 0)
            innerRadius = 0;

        // A. INEL EXTERIOR (Outline Negru)
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
            vertices.push_back(VertexFormat(pOut, black)); // Exterior
            vertices.push_back(VertexFormat(pIn, black));  // Interior
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

        // B. CERC INTERIOR (Colorat)
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

    // Calculăm cele 4 colțuri ale bazei și vârful piramidei
    float halfSize = baseSize / 2.0f;
    glm::vec3 b0 = baseCenter + glm::vec3(-halfSize, 0.0f, -halfSize); // Stanga-Spate
    glm::vec3 b1 = baseCenter + glm::vec3(halfSize, 0.0f, -halfSize);  // Dreapta-Spate
    glm::vec3 b2 = baseCenter + glm::vec3(halfSize, 0.0f, halfSize);   // Dreapta-Fata
    glm::vec3 b3 = baseCenter + glm::vec3(-halfSize, 0.0f, halfSize);  // Stanga-Fata
    glm::vec3 apex = baseCenter + glm::vec3(0.0f, height, 0.0f);       // Vârful

    if (!hasBlackBorder)
    {
        // Piramidă simplă fără outline
        unsigned int baseStart = vertices.size();

        // Baza (pătrat)
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

        // Vârful
        vertices.push_back(VertexFormat(apex, color));
        unsigned int apexIdx = vertices.size() - 1;

        // Cele 4 fețe laterale
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
        // Piramidă cu outline negru

        // 1. BAZA cu outline
        unsigned int baseStart = vertices.size();

        // Puncte interioare ale bazei (retragem cu thickness)
        glm::vec3 ib0 = b0 + glm::vec3(thickness, 0.0f, thickness);
        glm::vec3 ib1 = b1 + glm::vec3(-thickness, 0.0f, thickness);
        glm::vec3 ib2 = b2 + glm::vec3(-thickness, 0.0f, -thickness);
        glm::vec3 ib3 = b3 + glm::vec3(thickness, 0.0f, -thickness);

        // Miezul colorat al bazei
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

        // Rama neagră a bazei
        unsigned int frameStart = vertices.size();

        // Exterior (Negru)
        vertices.push_back(VertexFormat(b0, black)); // 0
        vertices.push_back(VertexFormat(b1, black)); // 1
        vertices.push_back(VertexFormat(b2, black)); // 2
        vertices.push_back(VertexFormat(b3, black)); // 3

        // Interior (Negru)
        vertices.push_back(VertexFormat(ib0, black)); // 4
        vertices.push_back(VertexFormat(ib1, black)); // 5
        vertices.push_back(VertexFormat(ib2, black)); // 6
        vertices.push_back(VertexFormat(ib3, black)); // 7

        // Triunghiuri ramă bază
        // Spate (0-1)
        indices.push_back(frameStart + 0);
        indices.push_back(frameStart + 1);
        indices.push_back(frameStart + 5);
        indices.push_back(frameStart + 0);
        indices.push_back(frameStart + 5);
        indices.push_back(frameStart + 4);
        // Dreapta (1-2)
        indices.push_back(frameStart + 1);
        indices.push_back(frameStart + 2);
        indices.push_back(frameStart + 6);
        indices.push_back(frameStart + 1);
        indices.push_back(frameStart + 6);
        indices.push_back(frameStart + 5);
        // Fata (2-3)
        indices.push_back(frameStart + 2);
        indices.push_back(frameStart + 3);
        indices.push_back(frameStart + 7);
        indices.push_back(frameStart + 2);
        indices.push_back(frameStart + 7);
        indices.push_back(frameStart + 6);
        // Stanga (3-0)
        indices.push_back(frameStart + 3);
        indices.push_back(frameStart + 0);
        indices.push_back(frameStart + 4);
        indices.push_back(frameStart + 3);
        indices.push_back(frameStart + 4);
        indices.push_back(frameStart + 7);

        // 2. FEȚELE LATERALE cu outline
        // Pentru fiecare față laterală: triunghi colorat interior + margini negre

        // Calculăm vârful interior (retras pe verticală și orizontal)
        glm::vec3 iApex = apex - glm::vec3(0.0f, thickness * 2.0f, 0.0f);

        auto CreateTriangleFace = [&](glm::vec3 outerA, glm::vec3 outerB, glm::vec3 innerA, glm::vec3 innerB)
        {
            unsigned int faceStart = vertices.size();

            // Triunghi colorat interior
            vertices.push_back(VertexFormat(innerA, color)); // 0
            vertices.push_back(VertexFormat(innerB, color)); // 1
            vertices.push_back(VertexFormat(iApex, color));  // 2

            indices.push_back(faceStart + 0);
            indices.push_back(faceStart + 1);
            indices.push_back(faceStart + 2);

            // Margini negre (3 benzi trapezoidale)
            unsigned int edgeStart = vertices.size();

            // Muchie bază (outerA -> outerB)
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

            // Muchie stângă (outerA -> apex)
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

            // Muchie dreaptă (outerB -> apex)
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

        // Cele 4 fețe laterale
        CreateTriangleFace(b0, b1, ib0, ib1); // Spate
        CreateTriangleFace(b1, b2, ib1, ib2); // Dreapta
        CreateTriangleFace(b2, b3, ib2, ib3); // Fata
        CreateTriangleFace(b3, b0, ib3, ib0); // Stanga
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
        // Sferă simplă fără outline
        unsigned int baseIdx = vertices.size();

        // Generăm vârfurile
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

        // Generăm indicii
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
        // Sferă cu outline negru
        float innerRadius = radius - thickness;
        if (innerRadius < 0)
            innerRadius = 0;

        unsigned int baseIdx = vertices.size();

        // Generăm vârfurile pentru sfera exterioară (neagră) și interioară (colorată)
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

                // Vârf exterior (negru)
                glm::vec3 outerPos = center + direction * radius;
                vertices.push_back(VertexFormat(outerPos, black));

                // Vârf interior (colorat)
                glm::vec3 innerPos = center + direction * innerRadius;
                vertices.push_back(VertexFormat(innerPos, color));
            }
        }

        // Generăm indicii pentru banda de outline și suprafața colorată
        for (int lat = 0; lat < latitudeSegments; lat++)
        {
            for (int lon = 0; lon < longitudeSegments; lon++)
            {
                int current = baseIdx + lat * (longitudeSegments + 1) * 2 + lon * 2;
                int next = current + (longitudeSegments + 1) * 2;

                // Banda de outline (între exterior și interior)
                // Quad: exterior_current -> exterior_next -> interior_next -> interior_current
                indices.push_back(current);  // exterior current
                indices.push_back(next);     // exterior next
                indices.push_back(next + 1); // interior next

                indices.push_back(current);     // exterior current
                indices.push_back(next + 1);    // interior next
                indices.push_back(current + 1); // interior current

                // Suprafața colorată interioară
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

// Cilindru orientat pe axa Y, colorat uniform
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

        // Fața laterală (două triunghiuri)
        indices.push_back(curr);
        indices.push_back(next);
        indices.push_back(curr + 1);

        indices.push_back(curr + 1);
        indices.push_back(next);
        indices.push_back(next + 1);
    }

    // Capace
    unsigned int centerBottom = vertices.size();
    vertices.push_back(VertexFormat(baseCenter, color));
    unsigned int centerTop = vertices.size();
    vertices.push_back(VertexFormat(baseCenter + glm::vec3(0.0f, height, 0.0f), color));

    for (int i = 0; i < segments; i++)
    {
        unsigned int curr = start + i * 2;
        unsigned int next = start + ((i + 1) % segments) * 2;

        // Baza
        indices.push_back(centerBottom);
        indices.push_back(next);
        indices.push_back(curr);

        // Capac superior
        indices.push_back(centerTop);
        indices.push_back(curr + 1);
        indices.push_back(next + 1);
    }
}

// Con orientat pe axa Y (baza în planul XZ, vârful în +Y)
void AddCone(std::vector<VertexFormat> &vertices, std::vector<unsigned int> &indices,
             glm::vec3 baseCenter, float radius, float height, glm::vec3 color,
             int segments = 24)
{
    glm::vec3 baseColor = color * 0.7f; // bază puțin mai închisă decât conul
    unsigned int baseStart = vertices.size();

    for (int i = 0; i < segments; i++)
    {
        float angle = 2.0f * M_PI * i / segments;
        float cosA = cos(angle);
        float sinA = sin(angle);
        glm::vec3 p = baseCenter + glm::vec3(radius * cosA, 0.0f, radius * sinA);
        vertices.push_back(VertexFormat(p, baseColor));
    }

    // Vârful
    unsigned int apexIndex = vertices.size();
    vertices.push_back(VertexFormat(baseCenter + glm::vec3(0.0f, height, 0.0f), color));

    // Laturile
    for (int i = 0; i < segments; i++)
    {
        unsigned int curr = baseStart + i;
        unsigned int next = baseStart + (i + 1) % segments;
        indices.push_back(curr);
        indices.push_back(next);
        indices.push_back(apexIndex);
    }

    // Centru bază
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

    // Trunchi
    float trunkHeight = 0.6f;
    float trunkRadius = 0.12f;
    AddCylinderY(vertices, indices, glm::vec3(0.0f), trunkRadius, trunkHeight, trunkColor, 24);

    // Frunze: 3 conuri identice, ușor decalate pe Y
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

    glm::vec3 baseColor(1.0f, 0.8f, 0.0f); // Galben
    glm::vec3 pinkColor(1.0f, 0.4f, 0.7f); // Roz

    // 1. Baza (Galben) - Platforma
    // Latime (Z) redusa la +/- 0.4.
    AddBox(vertices, indices, glm::vec3(-1.0f, 0.30f, -0.4f), glm::vec3(1.0f, 0.45f, 0.4f), baseColor, true);

    // 2. Cabina (Verde) - In spate
    // Sta pe baza (incepe de la 0.45)
    AddBox(vertices, indices, glm::vec3(-1.0f, 0.45f, -0.4f), glm::vec3(-0.2f, 1.15f, 0.4f), bodyColor, true);

    // 3. Motorul (Albastru) - CILINDRU orientat pe axa X
    AddCylinder(vertices, indices, glm::vec3(0.35f, 0.65f, 0.0f), 0.2f, 1.1f, cabinColor, 20, true, true);

    // 5. Conector (Roz) - CILINDRU mic lipit peste capătul motorului
    AddCylinder(vertices, indices, glm::vec3(0.96f, 0.65f, 0.0f), 0.08f, 0.12f, pinkColor, 20, true, true);

    // 6. Roti (Rosii) - CILINDRI orientați pe axa Z
    float wheelY = 0.15f;
    float wheelRadius = 0.15f;
    float wheelThickness = 0.12f;
    float zOffset = 0.3f; // Setat conform cerintei noi

    for (float x = -0.7f; x <= 0.7f; x += 0.35f)
    {
        // Roata Stânga - hasBlackTread = true
        AddCylinder(vertices, indices, glm::vec3(x, wheelY, zOffset), wheelRadius, wheelThickness, wheelColor, 20, false, false);
        // Roata Dreapta - hasBlackTread = true
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

    glm::vec3 baseColor(1.0f, 0.8f, 0.0f); // Galben

    // 1. Baza (Galben)
    // Coordonate: Y de la 0.30 la 0.45. Centrul pe Y este 0.375
    AddBox(vertices, indices, glm::vec3(-1.0f, 0.30f, -0.4f), glm::vec3(1.0f, 0.45f, 0.4f), baseColor, true);

    // 2. Corp (Verde)
    AddBox(vertices, indices, glm::vec3(-1.0f, 0.45f, -0.4f), glm::vec3(1.0f, 1.15f, 0.4f), bodyColor, true);

    // 3. Roti (Rosii) - 4 CILINDRI
    float wheelY = 0.15f;
    float wheelRadius = 0.15f;
    float wheelThickness = 0.12f;
    float zOffset = 0.3f;
    float xOffset = 0.7f;

    AddCylinder(vertices, indices, glm::vec3(xOffset, wheelY, zOffset), wheelRadius, wheelThickness, wheelColor, 20, false, false);
    AddCylinder(vertices, indices, glm::vec3(xOffset, wheelY, -zOffset), wheelRadius, wheelThickness, wheelColor, 20, false, false);
    AddCylinder(vertices, indices, glm::vec3(-xOffset, wheelY, zOffset), wheelRadius, wheelThickness, wheelColor, 20, false, false);
    AddCylinder(vertices, indices, glm::vec3(-xOffset, wheelY, -zOffset), wheelRadius, wheelThickness, wheelColor, 20, false, false);

    // --- 4. INEL CU GROSIME (Gri cu Gradient Negru) ---
    {
        glm::vec3 ringColor(0.5f, 0.5f, 0.5f); // Gri (Interior)
        glm::vec3 black(0.0f, 0.0f, 0.0f);     // Negru (Exterior)

        // Parametrii inelului
        glm::vec3 center(1.05f, 0.375f, 0.0f); // Pozitionare
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

            // 0: Sus-Interior -> Culoare: GRI (Centru)
            vertices.push_back(VertexFormat(
                glm::vec3(center.x + innerRadius * cosA, center.y + halfThick, center.z + innerRadius * sinA),
                ringColor));

            // 1: Sus-Exterior -> Culoare: NEGRU (Margine)
            vertices.push_back(VertexFormat(
                glm::vec3(center.x + outerRadius * cosA, center.y + halfThick, center.z + outerRadius * sinA),
                black));

            // 2: Jos-Interior -> Culoare: GRI (Centru)
            vertices.push_back(VertexFormat(
                glm::vec3(center.x + innerRadius * cosA, center.y - halfThick, center.z + innerRadius * sinA),
                ringColor));

            // 3: Jos-Exterior -> Culoare: NEGRU (Margine)
            vertices.push_back(VertexFormat(
                glm::vec3(center.x + outerRadius * cosA, center.y - halfThick, center.z + outerRadius * sinA),
                black));

            // Indicii (neschimbati)
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

            // Outer Wall (Negru complet)
            indices.push_back(c1);
            indices.push_back(c3);
            indices.push_back(n3);
            indices.push_back(c1);
            indices.push_back(n3);
            indices.push_back(n1);

            // Inner Wall (Gri complet)
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

    // 1. Platforma (mica)
    AddBox(vertices, indices, glm::vec3(-1.5f, 0.0f, -1.5f), glm::vec3(1.5f, 0.2f, 1.5f), platformColor, true);

    // 2. Casa (Cub simplu)
    AddBox(vertices, indices, glm::vec3(-1.0f, 0.2f, -1.0f), glm::vec3(1.0f, 1.5f, 1.0f), wallColor, true);

    // 3. Acoperis (Piramida cu outline)
    AddPyramid(vertices, indices, glm::vec3(0.0f, 1.5f, 0.0f), 2.4f, 1.0f, roofColor, true);

    // 4. Usa (Lipita de casa)
    AddBox(vertices, indices, glm::vec3(-0.3f, 0.2f, 1.0f), glm::vec3(0.3f, 1.2f, 1.05f), doorColor, true);

    // 5. Simbolul Plutitor (Deasupra acoperisului)
    // Acoperisul se termina la y=2.5. Punem simbolul sa inceapa de la y=3.05
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

    // 1. Platforma (identică cu BasicTrainStation)
    AddBox(vertices, indices, glm::vec3(-1.5f, 0.0f, -1.5f), glm::vec3(1.5f, 0.2f, 1.5f), platformColor, true);

    // 2. Cladirea Principala (aceleași dimensiuni ca casa din BasicTrainStation)
    AddBox(vertices, indices, glm::vec3(-1.0f, 0.2f, -1.0f), glm::vec3(1.0f, 1.5f, 1.0f), buildingColor, true);

    // 3. Acoperis (box în loc de piramidă, dar cu outline)
    AddBox(vertices, indices, glm::vec3(-1.2f, 1.5f, -1.2f), glm::vec3(1.2f, 1.8f, 1.2f), roofColor, true);

    // 4. Horn (Pe acoperis, cu outline)
    glm::vec3 chimneyColor(0.4f, 0.4f, 0.4f);
    AddBox(vertices, indices, glm::vec3(0.5f, 1.8f, 0.3f), glm::vec3(0.8f, 2.5f, 0.6f), chimneyColor, true);

    // 5. Usa (Lipită de casă, cu outline)
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
    // Lungimea vagonului/locomotivei: 2.0 unități (de la -1.0 la 1.0)
    // Ajustăm length să fie 2.0 pentru a se potrivi perfect
    length = 2.0f;

    // Poziția roților: zOffset = 0.3
    // Lățimea roților: wheelThickness = 0.12, deci ocupă de la -0.06 la +0.06 față de centru
    // Centrul roții la z = ±0.3
    // Roata stângă: de la 0.3 - 0.06 = 0.24 la 0.3 + 0.06 = 0.36
    // Roata dreaptă: de la -0.36 la -0.24

    float railWidth = 0.05f;     // Lățime șină
    float railHeight = 0.08f;    // Înălțime șină (puțin mai înaltă)
    float leftRailPos = 0.30f;   // Poziția șinei stângi (aliniată cu centrul roții)
    float rightRailPos = -0.30f; // Poziția șinei drepte (aliniată cu centrul roții)

    // Sina Stânga (aliniată cu roata stângă)
    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, leftRailPos - railWidth),
           glm::vec3(length / 2, railHeight, leftRailPos + railWidth),
           railColor);

    // Sina Dreapta (aliniată cu roata dreaptă)
    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, rightRailPos - railWidth),
           glm::vec3(length / 2, railHeight, rightRailPos + railWidth),
           railColor);

    // Traverse (Sleepers) - puse din loc în loc
    glm::vec3 sleeperColor(0.5f, 0.3f, 0.1f);
    int numSleepers = 5; // Număr fix pentru lungimea de 2.0
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
    // Lungimea ajustată pentru vagon/locomotivă
    length = 2.0f;

    // Poziția roților: zOffset = 0.3
    float railWidth = 0.05f;
    float railHeight = 0.08f; // puțin mai înalt
    float leftRailPos = 0.30f;
    float rightRailPos = -0.30f;

    // 1. Sinele (aliniate cu roțile)
    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, leftRailPos - railWidth),
           glm::vec3(length / 2, railHeight, leftRailPos + railWidth),
           railColor);

    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, rightRailPos - railWidth),
           glm::vec3(length / 2, railHeight, rightRailPos + railWidth),
           railColor);

    // 2. Tunelul - suficient de înalt și lat pentru locomotivă
    // Locomotiva: înălțime max ~1.45 (baza la 0.45 + cabina până la 1.15 + motor la 0.85)
    // Lățime: ±0.4 pentru corp
    float tunnelHeight = 2.0f; // Suficient de înalt
    float tunnelWidth = 1.2f;  // Suficient de lat
    float wallThickness = 0.3f;

    // Perete Stânga
    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, -tunnelWidth),
           glm::vec3(length / 2, tunnelHeight, -tunnelWidth + wallThickness),
           tunnelColor);

    // Perete Dreapta
    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, tunnelWidth - wallThickness),
           glm::vec3(length / 2, tunnelHeight, tunnelWidth),
           tunnelColor);

    // Tavan
    AddBox(vertices, indices,
           glm::vec3(-length / 2, tunnelHeight - wallThickness, -tunnelWidth),
           glm::vec3(length / 2, tunnelHeight, tunnelWidth),
           tunnelColor);

    // 3. Dungile albe (pe tavan, margini, lungime)
    glm::vec3 stripeColor(1.0f, 1.0f, 1.0f);
    float stripeWidth = 0.08f;
    float stripeThickness = 0.02f;

    // Dunga Stânga sus
    AddBox(vertices, indices,
           glm::vec3(-length / 2, tunnelHeight + 0.01f, -tunnelWidth + wallThickness),
           glm::vec3(length / 2, tunnelHeight + 0.01f + stripeThickness, -tunnelWidth + wallThickness + stripeWidth),
           stripeColor);

    // Dunga Dreapta sus
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
    // Lungimea ajustată pentru vagon/locomotivă
    length = 2.0f;

    // Poziția roților: zOffset = 0.3
    float railWidth = 0.05f;
    float railHeight = 0.08f; // puțin mai înalt
    float leftRailPos = 0.30f;
    float rightRailPos = -0.30f;

    // Podiul: ridicat ușor și mai subțire ca să nu iasă sub hartă
    float bridgeTopPlatform = 0.02f;
    float bridgeThickness = 0.06f; // grosime redusă

    // 1. Sinele (ridicate deasupra podului)
    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, leftRailPos - railWidth),
           glm::vec3(length / 2, 0.0f + railHeight, leftRailPos + railWidth),
           railColor);

    AddBox(vertices, indices,
           glm::vec3(-length / 2, 0.0f, rightRailPos - railWidth),
           glm::vec3(length / 2, 0.0f + railHeight, rightRailPos + railWidth),
           railColor);

    // 2. Podul (Platformă sub șine, suficient de lat)
    float bridgeWidth = 1.0f; // Lățime suficientă pentru stabilitate vizuală
    AddBox(vertices, indices,
           glm::vec3(-length / 2, bridgeTopPlatform - bridgeThickness, -bridgeWidth),
           glm::vec3(length / 2, bridgeTopPlatform, bridgeWidth),
           bridgeColor);

    // 3. Dungi albe transversale ("de mai multe ori pe segment")
    glm::vec3 stripeColor(1.0f, 1.0f, 1.0f);
    int numStripes = 6;
    float spacing = length / (numStripes + 1);

    for (int i = 1; i <= numStripes; i++)
    {
        float xPos = -length / 2 + i * spacing;
        // Dunga transversală pe pod
        AddBox(vertices, indices,
               glm::vec3(xPos - 0.03f, bridgeTopPlatform + 0.01f, -bridgeWidth * 0.9f),
               glm::vec3(xPos + 0.03f, bridgeTopPlatform + 0.02f, bridgeWidth * 0.9f),
               stripeColor);
    }

    Mesh *rail = new Mesh(name);
    rail->InitFromData(vertices, indices);
    return rail;
}

// ==================== INTERSECTION RAIL MESHES ====================
// These are special meshes for intersections where rails connect from multiple directions.
// Rails are only drawn on edges where connections exist (hasNorth, hasSouth, hasEast, hasWest).
// Directions: North (+Z), South (-Z), East (+X), West (-X)

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

    // Traverse (Sleepers) - in the center, always present
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

    // === RAIL LOGIC ===
    // For East-West direction: rails run along X axis at Z = ±railPos
    // For North-South direction: rails run along Z axis at X = ±railPos
    // Rails must extend to the edge of the mesh (halfLen) to connect with adjacent rail segments

    // East-West rails (Z = ±railPos, extending in X direction)
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

    // North-South rails (X = ±railPos, extending in Z direction)
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

    // === RAIL LOGIC (same as normal intersection) ===
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

    // === TUNNEL STRUCTURE ===
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

    // === RAIL LOGIC (same as normal intersection) ===
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