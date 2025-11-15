#include <glm/gtc/matrix_inverse.hpp>
#include "lab_m1/Arcade-Machine/Pong.hpp"
#include "lab_m1/lab3/transform2D.h"
#include "lab_m1/lab3/object2D.h"

using namespace m1;

glm::vec3 LerpColor(const glm::vec3 &a, const glm::vec3 &b, float t)
{
    return a * (1.0f - t) + b * t;
}

void Pong::SetShipLayout(const std::vector<ShipBlock> &blocks, float blockSize)
{
    shipLayout = blocks;
    shipBlockSize = blockSize;

    const ShipBlock &firstBlock = shipLayout.front();
    int minRow = firstBlock.row;
    int maxRow = firstBlock.row;
    int minCol = firstBlock.col;
    int maxCol = firstBlock.col;

    for (const auto &block : shipLayout)
    {
        UpdateExtentsForBlock(block, minRow, maxRow, minCol, maxCol);
    }

    shipMinRow = minRow;
    shipMinCol = minCol;
    shipWidthTiles = (maxCol - minCol + 1);
    shipHeightTiles = (maxRow - minRow + 1);
    shipWidthPx = shipWidthTiles * shipBlockSize;
    shipHeightPx = shipHeightTiles * shipBlockSize;

    float uniformScale = 1.0f;
    float paddleHeightScaled = 0.0f;
    float paddleThicknessScaled = 0.0f;
    if (ComputeShipScale(uniformScale, paddleHeightScaled, paddleThicknessScaled))
    {
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
}

void Pong::Init()
{
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)baseResolution.x, 0, (float)baseResolution.y, -1, 1);
    camera->SetPosition(glm::vec3(0, 0, 0));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();

    GetCameraInput()->SetActive(false);

    glm::ivec2 resolution = window->GetResolution();
    UpdateUITextScale(resolution);

    nextServeDirection = 1;

    delete textRenderer;
    textRenderer = new gfxc::TextRenderer(window->props.selfDir, resolution.x, resolution.y);

    const float baseFontPx = 28.0f;
    const unsigned int fontPixels = static_cast<unsigned int>(std::round(baseFontPx * uiTextScale));
    const unsigned int clampedFontPixels = std::max(16u, fontPixels);
    textRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, fontPath), clampedFontPixels);

    float bs = (shipBlockSize > 0.0f) ? shipBlockSize : 40.0f;
    EnsureBaseMeshes(bs);

    EnsureTintedMeshes("l", player1Color, bs);
    EnsureTintedMeshes("r", player2Color, bs);

    SpawnRoundBalls();
}

void Pong::FrameStart()
{
    glClearColor(0.16f, 0.16f, 0.16f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}

void Pong::Update(float deltaTimeSeconds)
{
    // If game over, just advance timer and draw end message
    if (winningPlayer != 0)
    {
        gameOverTimer += deltaTimeSeconds;
        // Clear background again to avoid leftover artifacts
        glClearColor(0.16f, 0.16f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        DrawEndMessage();
        if (gameOverTimer >= endMessageDuration)
        {
            window->Close();
        }
        return;
    }

    glm::mat3 model = glm::mat3(1);
    model *= transform2D::Translate(boardOffset.x, boardOffset.y);
    RenderMesh2D(meshes["board"], shaders["VertexColor"], model);

    float uniformScale = 1.0f;
    float paddleHeightScaled = playerSize.y;
    float paddleThicknessScaled = playerSize.x;
    bool shipRendered = false;
    glm::mat3 leftModel(1.0f);
    glm::mat3 rightModel(1.0f);

    if (!shipLayout.empty())
    {
        float computedScale = 1.0f;
        float computedHeight = 0.0f;
        float computedThickness = 0.0f;
        if (ComputeShipScale(computedScale, computedHeight, computedThickness))
        {
            uniformScale = computedScale;
            paddleHeightScaled = computedHeight;
            paddleThicknessScaled = computedThickness;

            leftModel = glm::mat3(1.0f);
            rightModel = glm::mat3(1.0f);

            leftModel *= transform2D::Translate(player1Offset.x + paddleThicknessScaled * 0.5f,
                                                player1Offset.y + paddleHeightScaled * 0.5f);
            leftModel *= transform2D::Rotate(glm::radians(90.0f));
            leftModel *= transform2D::Scale(uniformScale, uniformScale);
            RenderShip(leftModel, true);

            rightModel *= transform2D::Translate(player2Offset.x + paddleThicknessScaled * 0.5f,
                                                 player2Offset.y + paddleHeightScaled * 0.5f);
            rightModel *= transform2D::Rotate(glm::radians(270.0f));
            rightModel *= transform2D::Scale(uniformScale, uniformScale);
            RenderShip(rightModel, false);

            shipRendered = true;
        }
    }

    if (!shipRendered)
    {
        glm::mat3 paddleModel(1.0f);
        paddleModel *= transform2D::Translate(player1Offset.x, player1Offset.y);
        RenderMesh2D(meshes["player1"], shaders["VertexColor"], paddleModel);

        paddleModel = glm::mat3(1.0f);
        paddleModel *= transform2D::Translate(player2Offset.x, player2Offset.y);
        RenderMesh2D(meshes["player2"], shaders["VertexColor"], paddleModel);
    }

    for (const auto &ball : balls)
    {
        glm::mat3 ballModel(1.0f);
        ballModel *= transform2D::Translate(ball.pos.x, ball.pos.y);
        RenderMesh2D(meshes["ball"], shaders["VertexColor"], ballModel);
    }

    const float paddleCenterLeft = player1Offset.y + paddleHeightScaled * 0.5f;
    const float paddleCenterRight = player2Offset.y + paddleHeightScaled * 0.5f;

    for (auto &ball : balls)
    {
        if (shipRendered)
        {
            HandlePaddleCollision(ball, leftModel, uniformScale, paddleCenterLeft, true, paddleHeightScaled);
            HandlePaddleCollision(ball, rightModel, uniformScale, paddleCenterRight, false, paddleHeightScaled);
        }

        UpdateBall(ball, deltaTimeSeconds);
    }

    CheckScore();
    DrawScoreText();
}

void Pong::FrameEnd() {}
void Pong::OnInputUpdate(float deltaTime, int mods)
{
    float dy = paddleSpeed * deltaTime;
    float paddleHeightScaled = playerSize.y;
    if (window->KeyHold(GLFW_KEY_W))
        player1Offset.y = std::min(player1Offset.y + dy, boardOffset.y + boardSize.y - paddleHeightScaled);
    if (window->KeyHold(GLFW_KEY_S))
        player1Offset.y = std::max(player1Offset.y - dy, boardOffset.y);
    if (window->KeyHold(GLFW_KEY_UP))
        player2Offset.y = std::min(player2Offset.y + dy, boardOffset.y + boardSize.y - paddleHeightScaled);
    if (window->KeyHold(GLFW_KEY_DOWN))
        player2Offset.y = std::max(player2Offset.y - dy, boardOffset.y);
}
void Pong::OnKeyPress(int key, int mods) {}
void Pong::OnKeyRelease(int key, int mods) {}
void Pong::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) {}
void Pong::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) {}
void Pong::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) {}
void Pong::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) {}

// Mandatory for the TextRenderer to work properly upon window resize
void Pong::OnWindowResize(int width, int height)
{
    glm::ivec2 resolution(width, height);
    UpdateUITextScale(resolution);

    delete textRenderer;
    textRenderer = new gfxc::TextRenderer(window->props.selfDir, resolution.x, resolution.y);

    const float baseFontPx = 28.0f;
    const unsigned int fontPixels = static_cast<unsigned int>(std::round(baseFontPx * uiTextScale));
    const unsigned int clampedFontPixels = std::max(16u, fontPixels);
    textRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, fontPath), clampedFontPixels);
}

float Pong::MeasureTextWidth(const std::string &text, float scale) const
{
    if (!textRenderer)
        return 0.0f;
    float width = 0.0f;
    for (char c : text)
    {
        auto it = textRenderer->Characters.find(c);
        if (it == textRenderer->Characters.end())
            continue;
        width += ((it->second.Advance) >> 6) * scale;
    }
    return width;
}

void Pong::AccumulateTextMetrics(const std::string &text, float &maxBearing, float &maxDescent) const
{
    if (!textRenderer)
        return;

    for (char c : text)
    {
        auto it = textRenderer->Characters.find(c);
        if (it == textRenderer->Characters.end())
            continue;

        maxBearing = std::max(maxBearing, static_cast<float>(it->second.Bearing.y));
        float descent = static_cast<float>(it->second.Size.y - it->second.Bearing.y);
        maxDescent = std::max(maxDescent, descent);
    }
}

void Pong::UpdateUITextScale(const glm::ivec2 &resolution)
{
    if (resolution.y <= 0)
    {
        uiTextScale = 1.0f;
        return;
    }

    const float referenceHeight = static_cast<float>(baseResolution.y);
    if (referenceHeight <= 0.0f)
    {
        uiTextScale = 1.0f;
        return;
    }

    uiTextScale = static_cast<float>(resolution.y) / referenceHeight;
    if (uiTextScale < 0.35f)
        uiTextScale = 0.35f;
    else if (uiTextScale > 3.0f)
        uiTextScale = 3.0f;
}

void Pong::RenderShip(const glm::mat3 &baseModel, bool isLeftShip)
{
    for (const auto &pb : shipLayout)
    {
        float cx = ((pb.col - shipMinCol) + 0.5f) * shipBlockSize;
        float cy = ((pb.row - shipMinRow) + 0.5f) * shipBlockSize;

        glm::mat3 m = baseModel;
        m *= transform2D::Translate(-shipWidthPx / 2.0f, -shipHeightPx / 2.0f);

        switch (pb.type)
        {
        case SOLID:
        {
            float x = cx - shipBlockSize / 2.0f;
            float y = cy - shipBlockSize / 2.0f;
            glm::mat3 t = m;
            t *= transform2D::Translate(x, y);
            const char *meshName = isLeftShip ? "solid_tint_l" : "solid_tint_r";
            RenderMesh2D(meshes[meshName], shaders["VertexColor"], t);
            break;
        }
        case CANNON:
        {
            float x = cx - shipBlockSize / 2.0f;
            float y = cy - 1.5f * shipBlockSize;
            glm::mat3 t = m;
            t *= transform2D::Translate(x, y);
            const char *meshName = isLeftShip ? "cannon_tint_l" : "cannon_tint_r";
            RenderMesh2D(meshes[meshName], shaders["VertexColor"], t);
            break;
        }
        case BUMPER:
        {
            float x = cx;
            float y = cy + shipBlockSize / 2.0f;
            glm::mat3 t = m;
            t *= transform2D::Translate(x, y);
            const char *meshName = isLeftShip ? "bumper_tint_l" : "bumper_tint_r";
            RenderMesh2D(meshes[meshName], shaders["VertexColor"], t);
            break;
        }
        default:
            break;
        }
    }
}

bool Pong::ComputeShipScale(float &uniformScale, float &paddleHeightScaled, float &paddleThicknessScaled) const
{
    if (shipWidthPx <= 0.0f || shipHeightPx <= 0.0f)
        return false;

    float lengthScale = playerSize.y / shipWidthPx;
    float chosenScale = lengthScale;

    if (maxDisplayedShipWidthPx > 0.0f)
    {
        float widthScale = maxDisplayedShipWidthPx / shipHeightPx;
        if (widthScale > 0.0f)
            chosenScale = std::min(chosenScale, widthScale);
    }

    uniformScale = chosenScale;
    paddleHeightScaled = shipWidthPx * uniformScale;
    paddleThicknessScaled = shipHeightPx * uniformScale;
    return true;
}

void Pong::SpawnRoundBalls()
{
    balls.clear();

    if (!shipLayout.empty() && shipBlockSize > 0.0f)
    {
        float uniformScale = 1.0f;
        float paddleHeightScaled = 0.0f;
        float paddleThicknessScaled = 0.0f;
        if (ComputeShipScale(uniformScale, paddleHeightScaled, paddleThicknessScaled))
        {
            glm::mat3 leftModel(1.0f);
            leftModel *= transform2D::Translate(player1Offset.x + paddleThicknessScaled / 2.0f,
                                                player1Offset.y + paddleHeightScaled / 2.0f);
            leftModel *= transform2D::Rotate(glm::radians(90.0f));
            leftModel *= transform2D::Scale(uniformScale, uniformScale);

            glm::mat3 rightModel(1.0f);
            rightModel *= transform2D::Translate(player2Offset.x + paddleThicknessScaled / 2.0f,
                                                 player2Offset.y + paddleHeightScaled / 2.0f);
            rightModel *= transform2D::Rotate(glm::radians(270.0f));
            rightModel *= transform2D::Scale(uniformScale, uniformScale);

            SpawnBallsFromCannons(leftModel, true);
            SpawnBallsFromCannons(rightModel, false);
        }
    }

    if (balls.empty())
    {
        Ball ball{};
        ball.pos = glm::vec2(boardOffset.x + boardSize.x * 0.5f, boardOffset.y + boardSize.y * 0.5f);
        const float serveDir = (nextServeDirection >= 0) ? 1.0f : -1.0f;
        ball.dir = glm::vec2(serveDir, 0.0f);
        ball.launchGrace = 0.0f;
        ball.launchedFromLeft = (serveDir > 0.0f);
        ball.speedupCooldown = 0.0f;
        balls.push_back(ball);
    }
}

bool Pong::CollideBallWithShip(const glm::vec2 &ballCenter, float ballRadius, const glm::mat3 &baseModel, float uniformScale, bool &hitBumper)
{
    hitBumper = false;
    const glm::mat3 inv = glm::inverse(baseModel);
    const glm::vec3 local = inv * glm::vec3(ballCenter.x, ballCenter.y, 1.0f);
    const glm::vec2 center(local.x, local.y);
    const float radius = ballRadius / uniformScale;

    const float offsetX = -shipWidthPx * 0.5f;
    const float offsetY = -shipHeightPx * 0.5f;

    for (const auto &block : shipLayout)
    {
        const float cx = ((block.col - shipMinCol) + 0.5f) * shipBlockSize;
        const float cy = ((block.row - shipMinRow) + 0.5f) * shipBlockSize;

        switch (block.type)
        {
        case SOLID:
        {
            if (CircleIntersectsRect(center, radius,
                                     offsetX + cx - shipBlockSize * 0.5f,
                                     offsetY + cy - shipBlockSize * 0.5f,
                                     offsetX + cx + shipBlockSize * 0.5f,
                                     offsetY + cy + shipBlockSize * 0.5f))
            {
                return true;
            }
            break;
        }
        case CANNON:
        {
            for (int verticalOffset = -1; verticalOffset <= 1; ++verticalOffset)
            {
                const float scy = cy + verticalOffset * shipBlockSize;
                if (CircleIntersectsRect(center, radius,
                                         offsetX + cx - shipBlockSize * 0.5f,
                                         offsetY + scy - shipBlockSize * 0.5f,
                                         offsetX + cx + shipBlockSize * 0.5f,
                                         offsetY + scy + shipBlockSize * 0.5f))
                {
                    return true;
                }
            }
            break;
        }
        case BUMPER:
        {
            if (CircleIntersectsRect(center, radius,
                                     offsetX + cx - shipBlockSize * 0.5f,
                                     offsetY + cy - shipBlockSize * 0.5f,
                                     offsetX + cx + shipBlockSize * 0.5f,
                                     offsetY + cy + shipBlockSize * 0.5f))
            {
                hitBumper = true;
                return true;
            }

            const float ex = offsetX + cx;
            const float ey = offsetY + cy + shipBlockSize * 0.5f;
            const float rx = 1.5f * shipBlockSize;
            const float ry = shipBlockSize;
            if (CircleHitsBumperCap(center, radius, ex, ey, rx, ry))
            {
                hitBumper = true;
                return true;
            }
            break;
        }
        default:
            break;
        }
    }
    return false;
}

void Pong::DrawScoreText()
{
    if (!textRenderer)
        return;

    glm::ivec2 actualResolution = window->GetResolution();
    const float scaleX = static_cast<float>(actualResolution.x) / static_cast<float>(baseResolution.x);
    const float scaleY = static_cast<float>(actualResolution.y) / static_cast<float>(baseResolution.y);

    const float boardCenterScene = boardOffset.x + boardSize.x * 0.5f;
    const float boardCenterScreen = boardCenterScene * scaleX;
    const float boardTopScene = boardOffset.y + boardSize.y;
    const float boardTopScreen = boardTopScene * scaleY;
    const float gapScreen = std::max(0.0f, static_cast<float>(actualResolution.y) - boardTopScreen);

    std::string label = "SCORE";
    std::string sep = " : ";
    std::string sP1 = std::to_string(scoreP1);
    std::string sP2 = std::to_string(scoreP2);

    float labelW = MeasureTextWidth(label, uiTextScale);
    float sepW = MeasureTextWidth(sep, uiTextScale);
    float p1W = MeasureTextWidth(sP1, uiTextScale);
    float p2W = MeasureTextWidth(sP2, uiTextScale);

    const float spacing = 15.0f * uiTextScale;
    float totalW = labelW + spacing + p1W + sepW + p2W;
    float startX = boardCenterScreen - totalW * 0.5f;
    startX = std::max(0.0f, std::min(startX, static_cast<float>(actualResolution.x) - totalW));

    float maxBearing = 0.0f;
    float maxDescent = 0.0f;
    AccumulateTextMetrics(label, maxBearing, maxDescent);
    AccumulateTextMetrics(sep, maxBearing, maxDescent);
    AccumulateTextMetrics(sP1, maxBearing, maxDescent);
    AccumulateTextMetrics(sP2, maxBearing, maxDescent);

    float referenceBearing = 0.0f;
    auto refIt = textRenderer->Characters.find('H');
    if (refIt != textRenderer->Characters.end())
        referenceBearing = static_cast<float>(refIt->second.Bearing.y);

    if (maxBearing <= 0.0f)
        maxBearing = referenceBearing;

    const float textHeight = (maxBearing + maxDescent) * uiTextScale;
    float textY = 0.0f;
    if (gapScreen > textHeight)
        textY = (gapScreen - textHeight) * 0.5f;

    glm::vec3 white(1.0f);
    textRenderer->RenderText(label, startX, textY, uiTextScale, white);
    float cursor = startX + labelW + spacing;
    textRenderer->RenderText(sP1, cursor, textY, uiTextScale, player1Color);
    cursor += p1W;
    textRenderer->RenderText(sep, cursor, textY, uiTextScale, white);
    cursor += sepW;
    textRenderer->RenderText(sP2, cursor, textY, uiTextScale, player2Color);
}

void Pong::DrawEndMessage()
{
    if (!textRenderer)
        return;
    std::string msg = (winningPlayer == 1) ? "Player 1 win!" : "Player 2 win!";
    glm::vec3 color = (winningPlayer == 1) ? player1Color : player2Color;

    float scale = 1.5f * uiTextScale; // larger message, scaled with UI
    glm::ivec2 actualResolution = window->GetResolution();

    float msgW = MeasureTextWidth(msg, scale);
    float x = (static_cast<float>(actualResolution.x) - msgW) * 0.5f;
    x = std::max(0.0f, x);

    float maxBearing = 0.0f;
    float maxDescent = 0.0f;
    AccumulateTextMetrics(msg, maxBearing, maxDescent);

    float textHeight = (maxBearing + maxDescent) * scale;
    float y = (static_cast<float>(actualResolution.y) - textHeight) * 0.5f;
    y = std::max(0.0f, y);

    textRenderer->RenderText(msg, x, y, scale, color);
}

void Pong::ResetShipMetrics()
{
    shipMinCol = 0;
    shipMinRow = 0;
    shipWidthTiles = 0;
    shipHeightTiles = 0;
    shipWidthPx = 0.0f;
    shipHeightPx = 0.0f;
}

void Pong::UpdateCellExtents(int row, int col, int &minRow, int &maxRow, int &minCol, int &maxCol) const
{
    minRow = std::min(minRow, row);
    maxRow = std::max(maxRow, row);
    minCol = std::min(minCol, col);
    maxCol = std::max(maxCol, col);
}

void Pong::UpdateExtentsForBlock(const ShipBlock &block, int &minRow, int &maxRow, int &minCol, int &maxCol) const
{
    switch (block.type)
    {
    case SOLID:
        UpdateCellExtents(block.row, block.col, minRow, maxRow, minCol, maxCol);
        break;
    case CANNON:
        UpdateCellExtents(block.row - 1, block.col, minRow, maxRow, minCol, maxCol);
        UpdateCellExtents(block.row, block.col, minRow, maxRow, minCol, maxCol);
        UpdateCellExtents(block.row + 1, block.col, minRow, maxRow, minCol, maxCol);
        break;
    case BUMPER:
        UpdateCellExtents(block.row, block.col, minRow, maxRow, minCol, maxCol);
        UpdateCellExtents(block.row + 1, block.col, minRow, maxRow, minCol, maxCol);
        UpdateCellExtents(block.row + 1, block.col - 1, minRow, maxRow, minCol, maxCol);
        UpdateCellExtents(block.row + 1, block.col + 1, minRow, maxRow, minCol, maxCol);
        break;
    default:
        break;
    }
}

void Pong::EnsureBaseMeshes(float blockSize)
{
    if (meshes.find("board") == meshes.end())
    {
        AddMeshToList(object2D::CreateRectangle("board", glm::vec3(0.0f), boardSize.x, boardSize.y, glm::vec3(1.0f, 0.0f, 0.0f), false));
    }
    if (meshes.find("ball") == meshes.end())
    {
        AddMeshToList(object2D::CreateCircle("ball", glm::vec3(0.0f), ballSize, glm::vec3(1.0f), true));
    }
    if (meshes.find("solid") == meshes.end())
    {
        AddMeshToList(object2D::CreateSquare("solid", glm::vec3(0.0f), blockSize, solidColor, true));
    }
    if (meshes.find("cannon") == meshes.end())
    {
        AddMeshToList(object2D::CreateCannon("cannon", glm::vec3(0.0f), blockSize, cannonDarkColor, cannonLightColor, true));
    }
    if (meshes.find("bumper") == meshes.end())
    {
        AddMeshToList(object2D::CreateBumper("bumper", glm::vec3(0.0f), 1.5f * blockSize, blockSize, bumperColor, cannonLightColor, true));
    }
    if (meshes.find("player1") == meshes.end())
    {
        AddMeshToList(object2D::CreateRectangle("player1", glm::vec3(0.0f), playerSize.x, playerSize.y, player1Color, true));
    }
    if (meshes.find("player2") == meshes.end())
    {
        AddMeshToList(object2D::CreateRectangle("player2", glm::vec3(0.0f), playerSize.x, playerSize.y, player2Color, true));
    }
}

void Pong::EnsureTintedMeshes(const std::string &suffix, const glm::vec3 &tintColor, float blockSize)
{
    const std::string solidName = "solid_tint_" + suffix;
    if (meshes.find(solidName) == meshes.end())
    {
        const glm::vec3 solidTint = LerpColor(solidColor, tintColor, tintStrength);
        AddMeshToList(object2D::CreateSquare(solidName, glm::vec3(0.0f), blockSize, solidTint, true));
    }

    const std::string cannonName = "cannon_tint_" + suffix;
    if (meshes.find(cannonName) == meshes.end())
    {
        const glm::vec3 darkTint = LerpColor(cannonDarkColor, tintColor, tintStrength);
        const glm::vec3 lightTint = LerpColor(cannonLightColor, tintColor, tintStrength);
        AddMeshToList(object2D::CreateCannon(cannonName, glm::vec3(0.0f), blockSize, darkTint, lightTint, true));
    }

    const std::string bumperName = "bumper_tint_" + suffix;
    if (meshes.find(bumperName) == meshes.end())
    {
        const glm::vec3 semiTint = LerpColor(bumperColor, tintColor, tintStrength);
        const glm::vec3 squareTint = LerpColor(cannonLightColor, tintColor, tintStrength);
        AddMeshToList(object2D::CreateBumper(bumperName, glm::vec3(0.0f), 1.5f * blockSize, blockSize, semiTint, squareTint, true));
    }
}

void Pong::SpawnBallsFromCannons(const glm::mat3 &model, bool leftSide)
{
    for (const auto &block : shipLayout)
    {
        if (block.type != CANNON)
            continue;

        const float cx = ((block.col - shipMinCol) + 0.5f) * shipBlockSize;
        const float cy = ((block.row - shipMinRow) + 0.5f) * shipBlockSize;

        glm::mat3 localTransform(1.0f);
        localTransform *= transform2D::Translate(-shipWidthPx * 0.5f, -shipHeightPx * 0.5f);
        localTransform *= transform2D::Translate(cx, cy + shipBlockSize);

        const glm::mat3 worldTransform = model * localTransform;
        const glm::vec3 worldPos = worldTransform * glm::vec3(0.0f, 0.0f, 1.0f);

        Ball ball{};
        const glm::vec2 direction = leftSide ? glm::vec2(1.0f, 0.0f) : glm::vec2(-1.0f, 0.0f);
        glm::vec2 spawnPos(worldPos.x, worldPos.y);
        spawnPos += direction * (ballSize + 3.0f);
        const float lowerLimit = boardOffset.y + ballSize;
        const float upperLimit = boardOffset.y + boardSize.y - ballSize;
        if (spawnPos.y < lowerLimit)
            spawnPos.y = lowerLimit;
        else if (spawnPos.y > upperLimit)
            spawnPos.y = upperLimit;

        ball.pos = spawnPos;
        ball.dir = direction;
        ball.launchGrace = launchGraceDuration;
        ball.launchedFromLeft = leftSide;
        ball.speedupCooldown = 0.0f;
        balls.push_back(ball);
    }
}

void Pong::HandlePaddleCollision(Ball &ball, const glm::mat3 &model, float uniformScale, float paddleCenterY, bool leftSide, float paddleHeight)
{
    if (ball.launchGrace > 0.0f && (ball.launchedFromLeft == leftSide))
        return;

    bool hitBumper = false;
    if (!CollideBallWithShip(ball.pos, ballSize, model, uniformScale, hitBumper))
        return;

    const float halfHeight = paddleHeight * 0.5f;
    float relativeIntersect = (ball.pos.y - paddleCenterY) / halfHeight;
    if (relativeIntersect < -1.0f)
        relativeIntersect = -1.0f;
    else if (relativeIntersect > 1.0f)
        relativeIntersect = 1.0f;
    const float bounceAngle = relativeIntersect * glm::radians(60.0f);
    const float cosAngle = std::abs(std::cos(bounceAngle));

    ball.dir.x = leftSide ? cosAngle : -cosAngle;
    ball.dir.y = std::sin(bounceAngle);

    if (hitBumper && ball.speedupCooldown <= 0.0f)
    {
        ballSpeed = std::min(maxBallSpeed, ballSpeed * bumperSpeedFactor);
        ball.speedupCooldown = speedupCooldownInterval;
    }
}

void Pong::UpdateBall(Ball &ball, float deltaTimeSeconds)
{
    ball.pos += ball.dir * ballSpeed * deltaTimeSeconds;

    if (ball.launchGrace > 0.0f)
    {
        ball.launchGrace = std::max(0.0f, ball.launchGrace - deltaTimeSeconds);
    }
    if (ball.speedupCooldown > 0.0f)
    {
        ball.speedupCooldown = std::max(0.0f, ball.speedupCooldown - deltaTimeSeconds);
    }

    const float bottomLimit = boardOffset.y + ballSize;
    const float topLimit = boardOffset.y + boardSize.y - ballSize;

    if (ball.launchGrace <= 0.0f)
    {
        if (ball.pos.y < bottomLimit)
        {
            ball.pos.y = bottomLimit;
            ball.dir.y = std::abs(ball.dir.y);
        }
        else if (ball.pos.y > topLimit)
        {
            ball.pos.y = topLimit;
            ball.dir.y = -std::abs(ball.dir.y);
        }
    }
    else
    {
        if (ball.pos.y < bottomLimit)
            ball.pos.y = bottomLimit;
        else if (ball.pos.y > topLimit)
            ball.pos.y = topLimit;
    }
}

void Pong::CheckScore()
{
    int scoredSide = 0;
    for (const auto &ball : balls)
    {
        if (ball.pos.x < boardOffset.x)
        {
            scoredSide = -1;
            break;
        }
        if (ball.pos.x > boardOffset.x + boardSize.x)
        {
            scoredSide = 1;
            break;
        }
    }

    if (scoredSide == 0)
        return;

    if (scoredSide < 0)
        ++scoreP2;
    else
        ++scoreP1;

    ballSpeed = 400.0f;
    nextServeDirection = (scoredSide < 0) ? -1 : 1;
    SpawnRoundBalls();

    if (scoreP1 == numberOfRounds)
    {
        winningPlayer = 1;
        gameOverTimer = 0.0f;
    }
    else if (scoreP2 == numberOfRounds)
    {
        winningPlayer = 2;
        gameOverTimer = 0.0f;
    }
}

bool Pong::CircleIntersectsRect(const glm::vec2 &center, float radius, float rx0, float ry0, float rx1, float ry1) const
{
    const float clampedX = std::max(rx0, std::min(center.x, rx1));
    const float clampedY = std::max(ry0, std::min(center.y, ry1));
    const float dx = center.x - clampedX;
    const float dy = center.y - clampedY;
    return dx * dx + dy * dy <= radius * radius;
}

bool Pong::CircleHitsBumperCap(const glm::vec2 &center, float radius, float ex, float ey, float rx, float ry) const
{
    if (center.y < ey)
        return false;

    const float dx = (center.x - ex) / (rx + radius);
    const float dy = (center.y - ey) / (ry + radius);
    return dx * dx + dy * dy <= 1.0f;
}
