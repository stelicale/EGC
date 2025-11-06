#include <iostream>
#include <climits>
#include <unordered_map>
#include <algorithm>
#include <glm/gtc/matrix_inverse.hpp>
#include "lab_m1/Arcade-Machine/Pong.hpp"
#include "lab_m1/lab3/transform2D.h"
#include "lab_m1/lab3/object2D.h"

using namespace m1;

static glm::vec3 LerpColor(const glm::vec3 &a, const glm::vec3 &b, float t)
{
    float clamped = glm::clamp(t, 0.0f, 1.0f);
    return a * (1.0f - clamped) + b * clamped;
}

void Pong::ApplyPaddleAlignmentIfNeeded()
{
    if (!paddlesNeedAlignment)
        return;

    const float margin = 25.0f;

    if (hasShip && shipWidthPx > 0.0f && shipHeightPx > 0.0f)
    {
        float uniformScale = playerSize.y / shipWidthPx;
        float paddleHeightScaled = shipWidthPx * uniformScale;
        float paddleThicknessScaled = shipHeightPx * uniformScale;

        player1Offset = glm::vec3(boardOffset.x + margin,
                                  boardOffset.y + (boardSize.y - paddleHeightScaled) / 2.0f,
                                  0.0f);

        player2Offset = glm::vec3(boardOffset.x + boardSize.x - margin - paddleThicknessScaled,
                                  boardOffset.y + (boardSize.y - paddleHeightScaled) / 2.0f,
                                  0.0f);
    }
    else
    {
        player1Offset = glm::vec3(boardOffset.x + margin,
                                  boardOffset.y + (boardSize.y - playerSize.y) / 2.0f,
                                  0.0f);

        player2Offset = glm::vec3(boardOffset.x + boardSize.x - margin - playerSize.x,
                                  boardOffset.y + (boardSize.y - playerSize.y) / 2.0f,
                                  0.0f);
    }

    paddlesNeedAlignment = false;
}

void Pong::RenderShip(const glm::mat3 &baseModel, bool isLeftShip)
{
    // Choose side and whether we use tinted meshes
    const bool useTint = tintStrength > 0.0001f;
    const glm::vec3 tintColor = isLeftShip ? player1Color : player2Color;

    for (const auto &pb : shipLayout)
    {
        float cx = ((pb.col - shipMinCol) + 0.5f) * shipBlockSize;
        float cy = ((pb.row - shipMinRow) + 0.5f) * shipBlockSize;

        glm::mat3 m = baseModel;
        m *= transform2D::Translate(-shipWidthPx / 2.0f, -shipHeightPx / 2.0f);

        switch (pb.type)
        {
        case ShipBlockType::SOLID:
        {
            float x = cx - shipBlockSize / 2.0f;
            float y = cy - shipBlockSize / 2.0f;
            glm::mat3 t = m;
            t *= transform2D::Translate(x, y);
            if (useTint)
            {
                const char *meshName = isLeftShip ? "solid_tint_l" : "solid_tint_r";
                RenderMesh2D(meshes[meshName], shaders["VertexColor"], t);
            }
            else
            {
                RenderMesh2D(meshes["solid"], shaders["VertexColor"], t);
            }
            break;
        }
        case ShipBlockType::CANNON:
        {
            float x = cx - shipBlockSize / 2.0f;
            float y = cy - 1.5f * shipBlockSize;
            glm::mat3 t = m;
            t *= transform2D::Translate(x, y);
            if (useTint)
            {
                const char *meshName = isLeftShip ? "cannon_tint_l" : "cannon_tint_r";
                RenderMesh2D(meshes[meshName], shaders["VertexColor"], t);
            }
            else
            {
                RenderMesh2D(meshes["cannon"], shaders["VertexColor"], t);
            }
            break;
        }
        case ShipBlockType::BUMPER:
        {
            float x = cx;
            float y = cy + shipBlockSize / 2.0f;
            glm::mat3 t = m;
            t *= transform2D::Translate(x, y);
            if (useTint)
            {
                const char *meshName = isLeftShip ? "bumper_tint_l" : "bumper_tint_r";
                RenderMesh2D(meshes[meshName], shaders["VertexColor"], t);
            }
            else
            {
                RenderMesh2D(meshes["bumper"], shaders["VertexColor"], t);
            }
            break;
        }
        default:
            break;
        }
    }
}

void Pong::Init()
{
    // Editor-like: fixed logical orthographic projection (no deformation corrections here)
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)baseResolution.x, 0, (float)baseResolution.y, -1, 1);
    camera->SetPosition(glm::vec3(0, 0, 0));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();

    GetCameraInput()->SetActive(false);

    // Create meshes around origin; we'll position them in Update via transforms

    Mesh *board = object2D::CreateRectangle("board", glm::vec3(0, 0, 0), boardSize.x, boardSize.y, glm::vec3(1, 0, 0), false);
    AddMeshToList(board);
    Mesh *ball = object2D::CreateCircle("ball", glm::vec3(0, 0, 0), ballSize, glm::vec3(1, 1, 1), true);
    AddMeshToList(ball);

    // Ensure meshes for ship parts exist (solid, cannon, bumper) so they can render
    float bs = (shipBlockSize > 0.0f) ? shipBlockSize : 40.0f;
    if (meshes.find("solid") == meshes.end())
        AddMeshToList(object2D::CreateSquare("solid", glm::vec3(0, 0, 0), bs, solidColor, true));
    if (meshes.find("cannon") == meshes.end())
        AddMeshToList(object2D::CreateCannon("cannon", glm::vec3(0, 0, 0), bs, cannonDarkColor, cannonLightColor, true));
    if (meshes.find("bumper") == meshes.end())
        AddMeshToList(object2D::CreateBumper("bumper", glm::vec3(0, 0, 0), 1.5f * bs, bs, bumperColor, cannonLightColor, true));

    // Create tinted variants that preserve per-vertex colors by blending base colors with the tint color
    if (tintStrength > 0.0001f)
    {
        // Left tint meshes
        glm::vec3 lt = player1Color;
        glm::vec3 solid_lt = LerpColor(solidColor, lt, tintStrength);
        AddMeshToList(object2D::CreateSquare("solid_tint_l", glm::vec3(0, 0, 0), bs, solid_lt, true));
        glm::vec3 cannonDark_lt = LerpColor(cannonDarkColor, lt, tintStrength);
        glm::vec3 cannonLight_lt = LerpColor(cannonLightColor, lt, tintStrength);
        AddMeshToList(object2D::CreateCannon("cannon_tint_l", glm::vec3(0, 0, 0), bs, cannonDark_lt, cannonLight_lt, true));
        glm::vec3 bumperSemi_lt = LerpColor(bumperColor, lt, tintStrength);
        glm::vec3 bumperSquare_lt = LerpColor(cannonLightColor, lt, tintStrength);
        AddMeshToList(object2D::CreateBumper("bumper_tint_l", glm::vec3(0, 0, 0), 1.5f * bs, bs, bumperSemi_lt, bumperSquare_lt, true));

        // Right tint meshes
        glm::vec3 rt = player2Color;
        glm::vec3 solid_rt = LerpColor(solidColor, rt, tintStrength);
        AddMeshToList(object2D::CreateSquare("solid_tint_r", glm::vec3(0, 0, 0), bs, solid_rt, true));
        glm::vec3 cannonDark_rt = LerpColor(cannonDarkColor, rt, tintStrength);
        glm::vec3 cannonLight_rt = LerpColor(cannonLightColor, rt, tintStrength);
        AddMeshToList(object2D::CreateCannon("cannon_tint_r", glm::vec3(0, 0, 0), bs, cannonDark_rt, cannonLight_rt, true));
        glm::vec3 bumperSemi_rt = LerpColor(bumperColor, rt, tintStrength);
        glm::vec3 bumperSquare_rt = LerpColor(cannonLightColor, rt, tintStrength);
        AddMeshToList(object2D::CreateBumper("bumper_tint_r", glm::vec3(0, 0, 0), 1.5f * bs, bs, bumperSemi_rt, bumperSquare_rt, true));
    }

    // Base paddles kept for fallback/collision box
    Mesh *player1 = object2D::CreateRectangle("player1", glm::vec3(0, 0, 0), playerSize.x, playerSize.y, player1Color, true);
    AddMeshToList(player1);
    Mesh *player2 = object2D::CreateRectangle("player2", glm::vec3(0, 0, 0), playerSize.x, playerSize.y, player2Color, true);
    AddMeshToList(player2);

    Mesh *s = object2D::CreateLetterS("letterS", glm::vec3(0, 0, 0), 50, glm::vec3(1, 1, 1));
    AddMeshToList(s);
    Mesh *c = object2D::CreateLetterC("letterC", glm::vec3(0, 0, 0), 50, glm::vec3(1, 1, 1));
    AddMeshToList(c);
    Mesh *o = object2D::CreateLetterO("letterO", glm::vec3(0, 0, 0), 50, glm::vec3(1, 1, 1));
    AddMeshToList(o);
    Mesh *r = object2D::CreateLetterR("letterR", glm::vec3(0, 0, 0), 50, glm::vec3(1, 1, 1));
    AddMeshToList(r);
    Mesh *e = object2D::CreateLetterE("letterE", glm::vec3(0, 0, 0), 50, glm::vec3(1, 1, 1));
    AddMeshToList(e);

    // Create digits for both players with their respective colors
    Mesh *zeroP1 = object2D::CreateDigit0("digit0_p1", glm::vec3(0, 0, 0), 50, player1Color);
    AddMeshToList(zeroP1);
    Mesh *oneP1 = object2D::CreateDigit1("digit1_p1", glm::vec3(0, 0, 0), 50, player1Color);
    AddMeshToList(oneP1);
    Mesh *twoP1 = object2D::CreateDigit2("digit2_p1", glm::vec3(0, 0, 0), 50, player1Color);
    AddMeshToList(twoP1);

    Mesh *zeroP2 = object2D::CreateDigit0("digit0_p2", glm::vec3(0, 0, 0), 50, player2Color);
    AddMeshToList(zeroP2);
    Mesh *oneP2 = object2D::CreateDigit1("digit1_p2", glm::vec3(0, 0, 0), 50, player2Color);
    AddMeshToList(oneP2);
    Mesh *twoP2 = object2D::CreateDigit2("digit2_p2", glm::vec3(0, 0, 0), 50, player2Color);
    AddMeshToList(twoP2);

    Mesh *dots = object2D::CreateDots("dots", glm::vec3(0, 0, 0), 50, glm::vec3(1, 1, 1));
    AddMeshToList(dots);

    // Spawn initial round balls
    ApplyPaddleAlignmentIfNeeded();
    SpawnRoundBalls();
}

void Pong::FrameStart()
{
    // Clear screen at the beginning of each frame and set viewport to current window
    glClearColor(0.16, 0.16, 0.16, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}

// Helper to spawn balls from cannons at round start
void Pong::SpawnRoundBalls()
{
    ApplyPaddleAlignmentIfNeeded();

    balls.clear();

    // If we have a ship layout and block size, compute spawn positions for cannons
    if (hasShip && shipBlockSize > 0.0f && shipWidthPx > 0.0f && shipHeightPx > 0.0f)
    {
        // Precompute transforms for both paddles (same as render) to get world positions
        float uniformScale = playerSize.y / shipWidthPx;
        float paddleHeightScaled = shipWidthPx * uniformScale;
        float paddleThicknessScaled = shipHeightPx * uniformScale;

        glm::mat3 p1Model = glm::mat3(1);
        p1Model *= transform2D::Translate(player1Offset.x + paddleThicknessScaled / 2.0f,
                                          player1Offset.y + paddleHeightScaled / 2.0f);
        p1Model *= transform2D::Rotate(glm::radians(90.0f));
        p1Model *= transform2D::Scale(uniformScale, uniformScale);

        glm::mat3 p2Model = glm::mat3(1);
        p2Model *= transform2D::Translate(player2Offset.x + paddleThicknessScaled / 2.0f,
                                          player2Offset.y + paddleHeightScaled / 2.0f);
        p2Model *= transform2D::Rotate(glm::radians(270.0f));
        p2Model *= transform2D::Scale(uniformScale, uniformScale);

        auto addBallFromCannon = [&](const glm::mat3 &model, bool leftSide)
        {
            // Traverse layout and for each cannon place a ball at its muzzle (top of 3-stack before rotation)
            for (const auto &pb : shipLayout)
            {
                if (pb.type != ShipBlockType::CANNON)
                    continue;
                float cx = ((pb.col - shipMinCol) + 0.5f) * shipBlockSize;
                float cy = ((pb.row - shipMinRow) + 0.5f) * shipBlockSize;
                // Cannon muzzle (in local ship bbox space)
                float localX = cx;
                float localY = cy + shipBlockSize; // center of the top square in stack

                glm::mat3 full = model * transform2D::Translate(-shipWidthPx / 2.0f, -shipHeightPx / 2.0f) * transform2D::Translate(localX, localY);
                glm::vec3 wpos = full * glm::vec3(0, 0, 1);

                Ball b;
                glm::vec2 dir = leftSide ? glm::vec2(1.0f, 0.0f) : glm::vec2(-1.0f, 0.0f);
                // Offset a bit outside of paddle to avoid immediate collision
                glm::vec2 spawnPos = glm::vec2(wpos.x, wpos.y) + dir * (ballSize + 3.0f);
                spawnPos.y = glm::clamp(spawnPos.y,
                                        boardOffset.y + ballSize,
                                        boardOffset.y + boardSize.y - ballSize);
                b.pos = spawnPos;
                b.dir = dir; // strictly horizontal launch
                b.launchGrace = launchGraceDuration;
                b.launchedFromLeft = leftSide;
                b.speedupCooldown = 0.0f;
                balls.push_back(b);
            }
        };

        addBallFromCannon(p1Model, true);
        addBallFromCannon(p2Model, false);
    }

    // If no cannons or none spawned, add a single center ball towards right by default
    if (balls.empty())
    {
        Ball b;
        b.pos = glm::vec2(boardOffset.x + boardSize.x / 2.0f, boardOffset.y + boardSize.y / 2.0f);
        b.dir = glm::vec2(1.0f, 0.0f);
        b.launchGrace = 0.0f;
        b.launchedFromLeft = true;
        b.speedupCooldown = 0.0f;
        balls.push_back(b);
    }
}

void Pong::SetShipLayout(const std::vector<ShipBlock> &blocks, float blockSize)
{
    shipLayout = blocks;
    shipBlockSize = blockSize;
    hasShip = !shipLayout.empty() && shipBlockSize > 0.0f;
    paddlesNeedAlignment = true;

    if (!hasShip)
    {
        shipMinCol = shipMinRow = 0;
        shipWidthTiles = shipHeightTiles = 0;
        shipWidthPx = shipHeightPx = 0.0f;
        ApplyPaddleAlignmentIfNeeded();
        return;
    }

    // Compute tile bounds of the ship relative layout (min/max row/col)
    int minR = INT_MAX, maxR = INT_MIN, minC = INT_MAX, maxC = INT_MIN;
    auto markCell = [&](int r, int c)
    {
        if (r < minR)
            minR = r;
        if (r > maxR)
            maxR = r;
        if (c < minC)
            minC = c;
        if (c > maxC)
            maxC = c;
    };

    for (const auto &pb : shipLayout)
    {
        if (pb.type == ShipBlockType::SOLID)
        {
            markCell(pb.row, pb.col);
        }
        else if (pb.type == ShipBlockType::CANNON)
        {
            markCell(pb.row - 1, pb.col);
            markCell(pb.row, pb.col);
            markCell(pb.row + 1, pb.col);
        }
        else if (pb.type == ShipBlockType::BUMPER)
        {
            markCell(pb.row, pb.col);
            markCell(pb.row + 1, pb.col);
            markCell(pb.row + 1, pb.col - 1);
            markCell(pb.row + 1, pb.col + 1);
        }
    }

    if (minR == INT_MAX)
    {
        hasShip = false;
        return;
    }

    shipMinRow = minR;
    shipMinCol = minC;
    shipWidthTiles = (maxC - minC + 1);
    shipHeightTiles = (maxR - minR + 1);
    shipWidthPx = shipWidthTiles * shipBlockSize;
    shipHeightPx = shipHeightTiles * shipBlockSize;

    ApplyPaddleAlignmentIfNeeded();
}

bool Pong::CollideBallWithShip(const glm::vec2 &ballCenter, float ballRadius, const glm::mat3 &baseModel, float uniformScale, bool &hitBumper)
{
    hitBumper = false;

    // Transform ball center into ship-local bbox-centered space
    glm::mat3 inv = glm::inverse(baseModel);
    glm::vec3 local = inv * glm::vec3(ballCenter.x, ballCenter.y, 1.0f);
    glm::vec2 c(local.x, local.y);
    float r = ballRadius / uniformScale;

    auto circleRect = [&](float rx0, float ry0, float rx1, float ry1) -> bool
    {
        float nx = std::max(rx0, std::min(c.x, rx1));
        float ny = std::max(ry0, std::min(c.y, ry1));
        float dx = c.x - nx, dy = c.y - ny;
        return dx * dx + dy * dy <= r * r;
    };

    auto circleEllipseHalfTop = [&](float ex, float ey, float rx, float ry) -> bool
    {
        // Only top half: y >= ey
        if (c.y < ey)
            return false;
        float dx = (c.x - ex) / (rx + r);
        float dy = (c.y - ey) / (ry + r);
        return dx * dx + dy * dy <= 1.0f;
    };

    float ox = -shipWidthPx / 2.0f;
    float oy = -shipHeightPx / 2.0f;

    for (const auto &pb : shipLayout)
    {
        float cx = ((pb.col - shipMinCol) + 0.5f) * shipBlockSize;
        float cy = ((pb.row - shipMinRow) + 0.5f) * shipBlockSize;

        if (pb.type == ShipBlockType::SOLID)
        {
            if (circleRect(ox + cx - shipBlockSize / 2.0f,
                           oy + cy - shipBlockSize / 2.0f,
                           ox + cx + shipBlockSize / 2.0f,
                           oy + cy + shipBlockSize / 2.0f))
                return true;
        }
        else if (pb.type == ShipBlockType::CANNON)
        {
            for (int i = -1; i <= 1; ++i)
            {
                float scy = cy + i * shipBlockSize;
                if (circleRect(ox + cx - shipBlockSize / 2.0f,
                               oy + scy - shipBlockSize / 2.0f,
                               ox + cx + shipBlockSize / 2.0f,
                               oy + scy + shipBlockSize / 2.0f))
                    return true;
            }
        }
        else if (pb.type == ShipBlockType::BUMPER)
        {
            // square part
            if (circleRect(ox + cx - shipBlockSize / 2.0f,
                           oy + cy - shipBlockSize / 2.0f,
                           ox + cx + shipBlockSize / 2.0f,
                           oy + cy + shipBlockSize / 2.0f))
            {
                hitBumper = true;
                return true;
            }
            // semi-oval above
            float ex = ox + cx;
            float ey = oy + cy + shipBlockSize / 2.0f;
            float rx = 1.5f * shipBlockSize, ry = shipBlockSize;
            if (circleEllipseHalfTop(ex, ey, rx, ry))
            {
                hitBumper = true;
                return true;
            }
        }
    }
    return false;
}

void Pong::Update(float deltaTimeSeconds)
{
    ApplyPaddleAlignmentIfNeeded();

    // Board at boardOffset
    glm::mat3 model = glm::mat3(1);
    model *= transform2D::Translate(boardOffset.x, boardOffset.y);
    RenderMesh2D(meshes["board"], shaders["VertexColor"], model);

    float uniformScale = 1.0f;
    float paddleHeightScaled = playerSize.y;
    float paddleThicknessScaled = playerSize.x;

    // Render paddles: preserve aspect ratio; set vertical span (after rotation) == playerSize.y
    if (hasShip && shipWidthPx > 0.0f && shipHeightPx > 0.0f)
    {
        uniformScale = playerSize.y / shipWidthPx;           // target vertical length
        paddleHeightScaled = shipWidthPx * uniformScale;     // equals playerSize.y
        paddleThicknessScaled = shipHeightPx * uniformScale; // horizontal thickness after rotation

        // Player 1: rotate 90 deg CCW, blue side
        model = glm::mat3(1);
        model *= transform2D::Translate(player1Offset.x + paddleThicknessScaled / 2.0f,
                                        player1Offset.y + paddleHeightScaled / 2.0f);
        model *= transform2D::Rotate(glm::radians(90.0f));
        model *= transform2D::Scale(uniformScale, uniformScale);
        RenderShip(model, true);

        // Player 2: rotate 270 deg CCW, yellow side
        model = glm::mat3(1);
        model *= transform2D::Translate(player2Offset.x + paddleThicknessScaled / 2.0f,
                                        player2Offset.y + paddleHeightScaled / 2.0f);
        model *= transform2D::Rotate(glm::radians(270.0f));
        model *= transform2D::Scale(uniformScale, uniformScale);
        RenderShip(model, false);
    }
    else
    {
        // Fallback rectangles
        model = glm::mat3(1);
        model *= transform2D::Translate(player1Offset.x, player1Offset.y);
        RenderMesh2D(meshes["player1"], shaders["VertexColor"], model);

        model = glm::mat3(1);
        model *= transform2D::Translate(player2Offset.x, player2Offset.y);
        RenderMesh2D(meshes["player2"], shaders["VertexColor"], model);
    }

    // Render all balls
    for (const auto &b : balls)
    {
        glm::mat3 m = glm::mat3(1);
        m *= transform2D::Translate(b.pos.x, b.pos.y);
        RenderMesh2D(meshes["ball"], shaders["VertexColor"], m);
    }

    // Collisions with paddles using ship geometry if available
    if (hasShip)
    {
        glm::mat3 p1Model = glm::mat3(1);
        glm::mat3 p2Model = glm::mat3(1);

        p1Model *= transform2D::Translate(player1Offset.x + paddleThicknessScaled / 2.0f,
                                          player1Offset.y + paddleHeightScaled / 2.0f);
        p1Model *= transform2D::Rotate(glm::radians(90.0f));
        p1Model *= transform2D::Scale(uniformScale, uniformScale);

        p2Model *= transform2D::Translate(player2Offset.x + paddleThicknessScaled / 2.0f,
                                          player2Offset.y + paddleHeightScaled / 2.0f);
        p2Model *= transform2D::Rotate(glm::radians(270.0f));
        p2Model *= transform2D::Scale(uniformScale, uniformScale);

        for (auto &b : balls)
        {
            // Ignore collision with launching side while in launchGrace to keep the horizontal launch
            bool ignoreP1 = b.launchGrace > 0 && b.launchedFromLeft;
            bool ignoreP2 = b.launchGrace > 0 && !b.launchedFromLeft;

            bool hitBumper = false;
            if (!ignoreP1 && CollideBallWithShip(b.pos, ballSize, p1Model, uniformScale, hitBumper))
            {
                float paddleCenter = player1Offset.y + paddleHeightScaled / 2.0f;
                float relIntersect = (b.pos.y - paddleCenter) / (paddleHeightScaled / 2.0f);
                float bounceAngle = relIntersect * glm::radians(60.0f);
                b.dir.x = std::abs(cos(bounceAngle));
                b.dir.y = sin(bounceAngle);
                b.dir = glm::normalize(b.dir);
                if (hitBumper && b.speedupCooldown <= 0.0f)
                {
                    ballSpeed = std::min(maxBallSpeed, ballSpeed * bumperSpeedFactor);
                    b.speedupCooldown = speedupCooldownInterval;
                }
            }

            hitBumper = false;
            if (!ignoreP2 && CollideBallWithShip(b.pos, ballSize, p2Model, uniformScale, hitBumper))
            {
                float paddleCenter = player2Offset.y + paddleHeightScaled / 2.0f;
                float relIntersect = (b.pos.y - paddleCenter) / (paddleHeightScaled / 2.0f);
                float bounceAngle = relIntersect * glm::radians(60.0f);
                b.dir.x = -std::abs(cos(bounceAngle));
                b.dir.y = sin(bounceAngle);
                b.dir = glm::normalize(b.dir);
                if (hitBumper && b.speedupCooldown <= 0.0f)
                {
                    ballSpeed = std::min(maxBallSpeed, ballSpeed * bumperSpeedFactor);
                    b.speedupCooldown = speedupCooldownInterval;
                }
            }
        }
    }
    else
    {
        // No-ship legacy collisions not supported in multi-ball mode; skip
    }

    // Move balls
    for (auto &b : balls)
    {
        b.pos += b.dir * ballSpeed * deltaTimeSeconds;
        if (b.launchGrace > 0.0f)
        {
            b.launchGrace = std::max(0.0f, b.launchGrace - deltaTimeSeconds);
        }
        if (b.speedupCooldown > 0.0f)
        {
            b.speedupCooldown = std::max(0.0f, b.speedupCooldown - deltaTimeSeconds);
        }
        // top/bottom
        if (b.launchGrace <= 0.0f)
        {
            if (b.pos.y - ballSize < boardOffset.y)
            {
                b.pos.y = boardOffset.y + ballSize;
                b.dir.y = std::abs(b.dir.y);
            }
            if (b.pos.y + ballSize > boardOffset.y + boardSize.y)
            {
                b.pos.y = boardOffset.y + boardSize.y - ballSize;
                b.dir.y = -std::abs(b.dir.y);
            }
        }
        else
        {
            // During launch grace, clamp inside without changing direction to keep it horizontal
            if (b.pos.y - ballSize < boardOffset.y)
            {
                b.pos.y = boardOffset.y + ballSize;
            }
            if (b.pos.y + ballSize > boardOffset.y + boardSize.y)
            {
                b.pos.y = boardOffset.y + boardSize.y - ballSize;
            }
        }
    }

    // Check out of bounds; if any ball scores, reset round
    int scoredSide = 0; // -1 => left lost (score for P2), +1 => right lost (score for P1)
    for (const auto &b : balls)
    {
        if (b.pos.x < boardOffset.x)
        {
            scoredSide = -1;
            break;
        }
        if (b.pos.x > boardOffset.x + boardSize.x)
        {
            scoredSide = +1;
            break;
        }
    }
    if (scoredSide != 0)
    {
        if (scoredSide < 0)
            scoreP2++;
        else
            scoreP1++;
        // Reset speed and spawn next round
        ballSpeed = 400.0f;
        SpawnRoundBalls();
    }

    // Render "SCORE" text
    model = glm::mat3(1);
    model *= transform2D::Translate(baseResolution.x / 2 - 175, baseResolution.y - scoreOffset - 25);
    RenderMesh2D(meshes["letterS"], shaders["VertexColor"], model);
    model = glm::mat3(1);
    model *= transform2D::Translate(baseResolution.x / 2 - 125, baseResolution.y - scoreOffset - 25);
    RenderMesh2D(meshes["letterC"], shaders["VertexColor"], model);
    model = glm::mat3(1);
    model *= transform2D::Translate(baseResolution.x / 2 - 75, baseResolution.y - scoreOffset - 25);
    RenderMesh2D(meshes["letterO"], shaders["VertexColor"], model);
    model = glm::mat3(1);
    model *= transform2D::Translate(baseResolution.x / 2 - 25, baseResolution.y - scoreOffset - 25);
    RenderMesh2D(meshes["letterR"], shaders["VertexColor"], model);
    model = glm::mat3(1);
    model *= transform2D::Translate(baseResolution.x / 2 + 25, baseResolution.y - scoreOffset - 25);
    RenderMesh2D(meshes["letterE"], shaders["VertexColor"], model);

    // Scores
    std::string digitMeshP1 = "digit" + std::to_string(scoreP1) + "_p1";
    model = glm::mat3(1);
    model *= transform2D::Translate(baseResolution.x / 2 + 75, baseResolution.y - scoreOffset - 25);
    RenderMesh2D(meshes[digitMeshP1], shaders["VertexColor"], model);

    model = glm::mat3(1);
    model *= transform2D::Translate(baseResolution.x / 2 + 125, baseResolution.y - scoreOffset - 25);
    RenderMesh2D(meshes["dots"], shaders["VertexColor"], model);

    std::string digitMeshP2 = "digit" + std::to_string(scoreP2) + "_p2";
    model = glm::mat3(1);
    model *= transform2D::Translate(baseResolution.x / 2 + 150, baseResolution.y - scoreOffset - 25);
    RenderMesh2D(meshes[digitMeshP2], shaders["VertexColor"], model);

    if (scoreP1 == numberOfRounds)
    {
        std::cout << "\nPLAYER 1 WINS!\n"
                  << std::endl;
        window->Close();
    }
    if (scoreP2 == numberOfRounds)
    {
        std::cout << "\nPLAYER 2 WINS!\n"
                  << std::endl;
        window->Close();
    }
}

void Pong::FrameEnd()
{
}

void Pong::OnInputUpdate(float deltaTime, int mods)
{
    float dy = paddleSpeed * deltaTime;

    float uniformScale = 1.0f;
    float paddleHeightScaled = playerSize.y;
    if (hasShip && shipWidthPx > 0.0f && shipHeightPx > 0.0f)
    {
        uniformScale = playerSize.y / shipWidthPx;
        paddleHeightScaled = shipWidthPx * uniformScale;
    }

    if (window->KeyHold(GLFW_KEY_W))
    {
        player1Offset.y = std::min(player1Offset.y + dy, boardOffset.y + boardSize.y - paddleHeightScaled);
    }
    if (window->KeyHold(GLFW_KEY_S))
    {
        player1Offset.y = std::max(player1Offset.y - dy, boardOffset.y);
    }
    if (window->KeyHold(GLFW_KEY_UP))
    {
        player2Offset.y = std::min(player2Offset.y + dy, boardOffset.y + boardSize.y - paddleHeightScaled);
    }
    if (window->KeyHold(GLFW_KEY_DOWN))
    {
        player2Offset.y = std::max(player2Offset.y - dy, boardOffset.y);
    }
}

void Pong::OnKeyPress(int key, int mods)
{
}

void Pong::OnKeyRelease(int key, int mods)
{
}

void Pong::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
}

void Pong::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
}

void Pong::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}

void Pong::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Pong::OnWindowResize(int width, int height)
{
}
