#include <glm/gtc/matrix_inverse.hpp>
#include "lab_m1/Arcade-Machine/Editor.hpp"
#include "lab_m1/Arcade-Machine/Pong.hpp"
#include "lab_m1/lab3/transform2D.h"
#include "lab_m1/lab3/object2D.h"
#include <stack>

using namespace m1;

void Editor::Init()
{
    // Fix orthographic to a base logical resolution so content deforms with window aspect
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)baseResolution.x, 0, (float)baseResolution.y, -1, 1);
    camera->SetPosition(glm::vec3(0, 0, 0));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    Mesh *solid = object2D::CreateSquare("solid", glm::vec3(0, 0, 0), blockSize, solidColor, true);
    AddMeshToList(solid);
    Mesh *cannon = object2D::CreateCannon("cannon", glm::vec3(0, 0, 0), blockSize, cannonDarkColor, cannonLightColor, true);
    AddMeshToList(cannon);
    Mesh *bumper = object2D::CreateBumper("bumper", glm::vec3(0, 0, 0), blockSize * 1.5f, blockSize, bumperColor, cannonLightColor, true);
    AddMeshToList(bumper);

    Mesh *circle = object2D::CreateCircle("circle", glm::vec3(0, 0, 0), circleRadius, glm::vec3(0.5, 0, 0), true);
    AddMeshToList(circle);
    Mesh *tile = object2D::CreateSquare("tile", glm::vec3(0, 0, 0), tileSize, tileColor, true);
    AddMeshToList(tile);
    Mesh *part = object2D::CreateSquare("part", glm::vec3(0, 0, 0), tileSize, glm::vec3(1, 0, 0), false);
    AddMeshToList(part);
    Mesh *tileBoard = object2D::CreateRectangle("tileBoard", glm::vec3(0, 0, 0), boardW, boardH, tileBoardColor, false);
    AddMeshToList(tileBoard);
    Mesh *status = object2D::CreateSquare("status", glm::vec3(0, 0, 0), statusSize, statusColor, true);
    AddMeshToList(status);

    readyCircleColor = circleColor;
}

void Editor::FrameStart()
{
    if (inPongMode)
    {
        pong->FrameStart();
        return;
    }

    glClearColor(0.16f, 0.16f, 0.16f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Disable depth test for 2D rendering so draw order defines visibility
    glDisable(GL_DEPTH_TEST);

    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}

void Editor::Update(float deltaTimeSeconds)
{
    if (inPongMode)
    {
        pong->Update(deltaTimeSeconds);
        return;
    }

    UpdateReadyCircleColor();
    RenderReadyCircle();
    RenderStatusBar();
    RenderLeftPanelBlocks();
    RenderTileGrid();
    RenderPlacedBlocks();
    RenderDraggedBlock();
}

void Editor::FrameEnd()
{
    if (inPongMode)
    {
        pong->FrameEnd();
        return;
    }
}

void Editor::OnInputUpdate(float deltaTime, int mods)
{
    if (inPongMode)
    {
        pong->OnInputUpdate(deltaTime, mods);
        return;
    }
}

void Editor::OnKeyPress(int key, int mods)
{
    if (inPongMode)
    {
        pong->OnKeyPress(key, mods);
        return;
    }
}

void Editor::OnKeyRelease(int key, int mods)
{
    if (inPongMode)
    {
        pong->OnKeyRelease(key, mods);
        return;
    }
}

void Editor::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    if (inPongMode)
    {
        pong->OnMouseMove(mouseX, mouseY, deltaX, deltaY);
        return;
    }

    if (draggedBlock != NONE)
    {
        glm::vec2 scenePos = screenToScene(mouseX, mouseY);
        dragPosition = scenePos - dragOffset;
    }
}

void Editor::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    if (inPongMode)
    {
        pong->OnMouseBtnPress(mouseX, mouseY, button, mods);
        return;
    }

    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT))
    {
        // Check if clicking on ready circle
        if (checkReady(mouseX, mouseY))
        {
            if (!pong)
                pong = new Pong();
            // Pass to pong BEFORE Init to ensure meshes exist when it starts
            std::vector<Pong::ShipBlock> ship;
            ship.reserve(placedBlocks.size());
            for (const auto &pb : placedBlocks)
            {
                if (pb.type == NONE)
                    continue;

                Pong::ShipBlock sb;
                sb.row = pb.row;
                sb.col = pb.col;
                sb.type = static_cast<Pong::ShipBlockType>(pb.type - 1);
                ship.push_back(sb);
            }
            pong->SetShipLayout(ship, blockSize);
            pong->Init();
            inPongMode = true;
            return;
        }

        // Check if clicking on any draggable block
        glm::vec2 scenePos = screenToScene(mouseX, mouseY);
        BlockType block = getBlockAtPosition(scenePos);

        if (block != NONE)
        {
            draggedBlock = block;
            // Calculate offset from block center to mouse position
            float blockCenterX, blockCenterY;
            switch (block)
            {
            case SOLID:
                blockCenterX = solidX;
                blockCenterY = solidY;
                dragAnchor = 0; // not used
                break;
            case CANNON:
            {
                blockCenterX = cannonX;
                blockCenterY = cannonY;
                float relY = scenePos.y - blockCenterY; // 0..3*blockSize
                if (relY < blockSize)
                    dragAnchor = 0; // bottom
                else if (relY < 2 * blockSize)
                    dragAnchor = 1; // middle
                else
                    dragAnchor = 2; // top
                break;
            }
            case BUMPER:
            {
                blockCenterX = bumperX;
                blockCenterY = bumperY;
                dragAnchor = (scenePos.y <= blockCenterY) ? 0 : 1; // 0=square, 1=semioval
                break;
            }
            default:
                blockCenterX = blockCenterY = 0;
            }
            dragOffset = scenePos - glm::vec2(blockCenterX, blockCenterY);
            dragPosition = glm::vec2(blockCenterX, blockCenterY);
        }
    }
}

void Editor::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    if (inPongMode)
    {
        pong->OnMouseBtnRelease(mouseX, mouseY, button, mods);
        return;
    }

    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT))
    {
        if (draggedBlock != NONE)
        {
            if (statusRemaining <= 0)
            {
                ResetDragState();
                return;
            }

            const glm::vec2 scenePos = screenToScene(mouseX, mouseY);
            if (TryPlaceDraggedBlock(scenePos))
            {
                statusRemaining = std::max(0, statusRemaining - 1);
            }

            ResetDragState();
        }
    }

    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_RIGHT))
    {
        const glm::vec2 scenePos = screenToScene(mouseX, mouseY);
        RemoveBlockAtPosition(scenePos);
    }
}

void Editor::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    if (inPongMode)
    {
        pong->OnMouseScroll(mouseX, mouseY, offsetX, offsetY);
        return;
    }
}

void Editor::OnWindowResize(int width, int height) {}

bool Editor::checkReady(int mouseX, int mouseY)
{
    // Ready only when placement is valid (green circle)
    if (readyCircleColor != glm::vec3(0, 0.5, 0))
        return false;

    glm::vec2 scenePos = screenToScene(mouseX, mouseY);
    glm::vec2 diff = scenePos - circleCenter;
    float distSquared = diff.x * diff.x + diff.y * diff.y;
    return distSquared <= circleRadius * circleRadius;
}

glm::vec2 Editor::screenToScene(int mouseX, int mouseY)
{
    // Map window-space to logical scene-space using current viewport size and base resolution
    glm::ivec2 res = window->GetResolution();
    if (res.x <= 0 || res.y <= 0)
        return glm::vec2(0, 0);

    float invResX = 1.0f / res.x;
    float invResY = 1.0f / res.y;
    float sceneX = mouseX * invResX * baseResolution.x;
    float sceneY = (res.y - mouseY) * invResY * baseResolution.y;

    return glm::vec2(sceneX, sceneY);
}

Editor::BlockType Editor::getBlockAtPosition(glm::vec2 scenePos)
{
    // Check solid block (first part) - standard square
    float solidX = margin + partSize.x / 2 - blockSize / 2;
    float solidY = margin + partSize.y / 2 - blockSize / 2;
    if (scenePos.x >= solidX && scenePos.x <= solidX + blockSize &&
        scenePos.y >= solidY && scenePos.y <= solidY + blockSize)
    {
        return SOLID;
    }

    // Check cannon block (second part) - 3 squares tall (total height = 3 * blockSize)
    float cannonX = margin + partSize.x / 2 - blockSize / 2;
    float cannonY = margin + partSize.y + partSize.y / 2 - 3 * blockSize / 2;
    if (scenePos.x >= cannonX && scenePos.x <= cannonX + blockSize &&
        scenePos.y >= cannonY && scenePos.y <= cannonY + 3 * blockSize)
    {
        return CANNON;
    }

    // Check bumper block (third part) - precise hitbox matching rendered pixels
    float bumperCenterX = margin + partSize.x / 2;
    float bumperCenterY = margin + 2 * partSize.y + partSize.y / 2;

    // Check square part first (bottom part of bumper)
    float squareLeft = bumperCenterX - blockSize / 2;
    float squareRight = bumperCenterX + blockSize / 2;
    float squareBottom = bumperCenterY - blockSize;
    float squareTop = bumperCenterY;

    if (scenePos.x >= squareLeft && scenePos.x <= squareRight &&
        scenePos.y >= squareBottom && scenePos.y <= squareTop)
    {
        return BUMPER;
    }

    // Check semioval part (top part of bumper) - elliptical top half
    // Only check if y >= bumperCenterY (top half)
    if (scenePos.y >= bumperCenterY)
    {
        float ovalRadiusX = 1.5f * blockSize;
        float ovalRadiusY = blockSize;

        // Normalized distance in ellipse coordinates
        float dx = (scenePos.x - bumperCenterX) / ovalRadiusX;
        float dy = (scenePos.y - bumperCenterY) / ovalRadiusY;

        // Check if point is inside the ellipse
        if (dx * dx + dy * dy <= 1.0f)
        {
            return BUMPER;
        }
    }

    return NONE;
}

bool Editor::getTileAtPosition(glm::vec2 scenePos, int &row, int &col) const
{
    // Check if position is within the tile grid bounds
    for (int r = 0; r < tilesPerCol; r++)
    {
        for (int c = 0; c < tilesPerRow; c++)
        {
            if (scenePos.x >= tiles[r][c].downLeft.x && scenePos.x <= tiles[r][c].upRight.x &&
                scenePos.y >= tiles[r][c].downLeft.y && scenePos.y <= tiles[r][c].upRight.y)
            {
                row = r;
                col = c;
                return true;
            }
        }
    }
    return false;
}

glm::vec2 Editor::getTileCenter(int row, int col) const
{
    // Calculate the position where blocks should be placed on this tile
    // Each block type has different reference points, so we center them on the tile

    float tileX = tiles[row][col].downLeft.x;
    float tileY = tiles[row][col].downLeft.y;
    float tileCenterX = tileX + tileSize / 2;
    float tileCenterY = tileY + tileSize / 2;

    // Return center of tile - blocks will be adjusted based on their type during rendering
    return glm::vec2(tileCenterX, tileCenterY);
}

bool Editor::correctPlacement()
{
    const int rows = tilesPerCol;
    const int cols = tilesPerRow;
    int total = 0;
    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            if (occupancy[r][c])
            {
                ++total;
            }
        }
    }
    if (total == 0)
        return false;
    int midR = tilesPerCol / 2;
    int midC = tilesPerRow / 2;
    if (midR < 0 || midR >= rows || midC < 0 || midC >= cols)
        return false;
    // Mandatory center tile must be filled so the constructed ship has its core.
    if (!occupancy[midR][midC])
        return false;
    // DFS
    std::stack<std::pair<int, int>> stk;
    std::vector<std::vector<int>> vis(rows, std::vector<int>(cols, 0));
    for (int r = 0; r < rows; ++r)
    {
        bool found = false;
        for (int c = 0; c < cols; ++c)
        {
            if (occupancy[r][c])
            {
                stk.push({r, c});
                vis[r][c] = 1;
                found = true;
                break;
            }
        }
        if (found)
            break;
    }
    int visited = 0;
    const int dr[4] = {1, -1, 0, 0};
    const int dc[4] = {0, 0, 1, -1};
    while (!stk.empty())
    {
        auto cur = stk.top();
        stk.pop();
        int r = cur.first;
        int c = cur.second;
        ++visited;
        for (int k = 0; k < 4; ++k)
        {
            int nr = r + dr[k], nc = c + dc[k];
            if (nr < 0 || nr >= rows || nc < 0 || nc >= cols)
                continue;
            if (!occupancy[nr][nc] || vis[nr][nc])
                continue;
            vis[nr][nc] = 1;
            stk.push({nr, nc});
        }
    }
    return visited == total;
}

bool Editor::violatesAboveRestrictions(const std::vector<std::pair<int, int>> &cells) const
{
    // For each candidate cell, ensure it's not strictly above an existing bumper/cannon in their restricted columns
    for (const auto &pb : placedBlocks)
    {
        if (pb.type == BUMPER)
        {
            int topRow = pb.row + 1;
            int colsB[3] = {pb.col - 1, pb.col, pb.col + 1};
            for (const auto &cell : cells)
            {
                int r = cell.first;
                int c = cell.second;
                if (r > topRow && (c == colsB[0] || c == colsB[1] || c == colsB[2]))
                    return true;
            }
        }
        else if (pb.type == CANNON)
        {
            int topRow = pb.row + 1; // pb.row is middle
            for (const auto &cell : cells)
            {
                int r = cell.first;
                int c = cell.second;
                if (r > topRow && c == pb.col)
                    return true;
            }
        }
    }
    return false;
}

bool Editor::noBlocksAboveInRestrictedColumns(int topRow, int colMin, int colMax) const
{
    // Ensure no occupied cells strictly above topRow in [colMin..colMax]
    for (int c = colMin; c <= colMax; ++c)
    {
        if (c < 0 || c >= tilesPerRow)
            continue;
        for (int r = topRow + 1; r < tilesPerCol; ++r)
        {
            if (occupancy[r][c])
                return false;
        }
    }
    return true;
}

void Editor::updateOccupancyForBlock(const PlacedBlock &block, int value)
{

    switch (block.type)
    {
    case SOLID:
        occupancy[block.row][block.col] = value;
        break;
    case CANNON:
        occupancy[block.row - 1][block.col] = value;
        occupancy[block.row][block.col] = value;
        occupancy[block.row + 1][block.col] = value;
        break;
    case BUMPER:
        occupancy[block.row][block.col] = value;
        occupancy[block.row + 1][block.col] = value;
        occupancy[block.row + 1][block.col - 1] = value;
        occupancy[block.row + 1][block.col + 1] = value;
        break;
    default:
        break;
    }
}

void Editor::UpdateReadyCircleColor()
{
    readyCircleColor = correctPlacement() ? glm::vec3(0.0f, 0.5f, 0.0f) : glm::vec3(0.5f, 0.0f, 0.0f);
}

void Editor::RenderReadyCircle()
{
    glm::mat3 model(1.0f);
    model *= transform2D::Translate(circleCenter.x, circleCenter.y);
    RenderMesh2D(meshes["circle"], model, readyCircleColor);
}

void Editor::RenderStatusBar()
{
    for (int i = 0; i < statusRemaining; ++i)
    {
        RenderStatusSquare(i);
    }
}

void Editor::RenderStatusSquare(int index)
{
    const float statusStartX = margin + partSize.x + 2.0f * margin;
    const float statusY = circleCenter.y - statusSize * 0.5f;
    const float x = statusStartX + index * (statusSize + 2.0f * statusSpacing);

    if (index >= 0 && index < static_cast<int>(statusParts.size()))
    {
        statusParts[index].downLeft = glm::vec2(x, statusY);
        statusParts[index].upRight = glm::vec2(x + statusSize, statusY + statusSize);
    }

    glm::mat3 statusModel(1.0f);
    statusModel *= transform2D::Translate(x, statusY);
    RenderMesh2D(meshes["status"], shaders["VertexColor"], statusModel);
}

void Editor::RenderLeftPanelBlocks()
{
    for (int i = 0; i < partCount; ++i)
    {
        const float y = margin + i * partSize.y;
        glm::mat3 partModel(1.0f);
        partModel *= transform2D::Translate(margin, y);
        partModel *= transform2D::Scale(partSize.x / tileSize, partSize.y / tileSize);
        RenderMesh2D(meshes["part"], shaders["VertexColor"], partModel);
    }

    glm::mat3 model(1.0f);
    model *= transform2D::Translate(solidX, solidY);
    RenderMesh2D(meshes["solid"], shaders["VertexColor"], model);

    model = glm::mat3(1.0f);
    model *= transform2D::Translate(cannonX, cannonY);
    RenderMesh2D(meshes["cannon"], shaders["VertexColor"], model);

    model = glm::mat3(1.0f);
    model *= transform2D::Translate(bumperX, bumperY);
    RenderMesh2D(meshes["bumper"], shaders["VertexColor"], model);
}

void Editor::RenderTileGrid()
{
    glm::mat3 boardModel(1.0f);
    boardModel *= transform2D::Translate(boardX, boardY);
    RenderMesh2D(meshes["tileBoard"], shaders["VertexColor"], boardModel);

    const float startX = boardX + margin;
    const float startY = boardY + margin;
    const int mandatoryRow = tilesPerCol / 2;
    const int mandatoryCol = tilesPerRow / 2;

    for (int r = 0; r < tilesPerCol; ++r)
    {
        for (int c = 0; c < tilesPerRow; ++c)
        {
            const float x = startX + c * (tileSize + 2.0f * tileSpacing);
            const float y = startY + r * (tileSize + 2.0f * tileSpacing);

            tiles[r][c].downLeft = glm::vec2(x, y);
            tiles[r][c].upRight = glm::vec2(x + tileSize, y + tileSize);

            glm::mat3 tileModel(1.0f);
            tileModel *= transform2D::Translate(x, y);

            if (r == mandatoryRow && c == mandatoryCol)
            {
                RenderMesh2D(meshes["tile"], tileModel, mandatoryTileColor);
            }
            else
            {
                RenderMesh2D(meshes["tile"], shaders["VertexColor"], tileModel);
            }
        }
    }
}

void Editor::RenderPlacedBlocks()
{
    for (const auto &block : placedBlocks)
    {
        RenderBlockMeshes(block);
    }
}

void Editor::RenderBlockMeshes(const PlacedBlock &block)
{
    glm::mat3 model(1.0f);

    switch (block.type)
    {
    case BlockType::SOLID:
    {
        const glm::vec2 center = getTileCenter(block.row, block.col);
        model *= transform2D::Translate(center.x - blockSize * 0.5f, center.y - blockSize * 0.5f);
        RenderMesh2D(meshes["solid"], shaders["VertexColor"], model);
        break;
    }
    case BlockType::CANNON:
    {
        const glm::vec2 middleCenter = getTileCenter(block.row, block.col);
        const float baseX = middleCenter.x - blockSize * 0.5f;
        const float baseY = middleCenter.y - 1.5f * blockSize;
        model *= transform2D::Translate(baseX, baseY);
        RenderMesh2D(meshes["cannon"], shaders["VertexColor"], model);
        break;
    }
    case BlockType::BUMPER:
    {
        const glm::vec2 squareCenter = getTileCenter(block.row, block.col);
        model *= transform2D::Translate(squareCenter.x, squareCenter.y + blockSize * 0.5f);
        RenderMesh2D(meshes["bumper"], shaders["VertexColor"], model);
        break;
    }
    default:
        break;
    }
}

void Editor::RenderDraggedBlock()
{
    if (draggedBlock == NONE)
        return;

    glm::mat3 model(1.0f);
    model *= transform2D::Translate(dragPosition.x, dragPosition.y);

    switch (draggedBlock)
    {
    case SOLID:
        RenderMesh2D(meshes["solid"], shaders["VertexColor"], model);
        break;
    case CANNON:
        RenderMesh2D(meshes["cannon"], shaders["VertexColor"], model);
        break;
    case BUMPER:
        RenderMesh2D(meshes["bumper"], shaders["VertexColor"], model);
        break;
    default:
        break;
    }
}

bool Editor::TryPlaceDraggedBlock(const glm::vec2 &scenePos)
{
    if (draggedBlock == NONE)
        return false;

    PlacedBlock candidate;
    candidate.type = draggedBlock;
    candidate.anchor = dragAnchor;

    if (draggedBlock == BUMPER)
    {
        const glm::vec2 dropOrigin = scenePos - dragOffset;
        if (TryPlaceBumper(dropOrigin, candidate))
        {
            CommitBlockPlacement(candidate);
            return true;
        }
        return false;
    }

    int row = 0;
    int col = 0;
    if (!getTileAtPosition(scenePos, row, col))
        return false;

    if (draggedBlock == SOLID)
    {
        if (TryPlaceSolid(row, col, candidate))
        {
            CommitBlockPlacement(candidate);
            return true;
        }
        return false;
    }

    if (draggedBlock == CANNON && TryPlaceCannon(row, col, candidate))
    {
        CommitBlockPlacement(candidate);
        return true;
    }

    return false;
}

bool Editor::TryPlaceSolid(int row, int col, PlacedBlock &outBlock) const
{
    if (row < 0 || row >= tilesPerCol || col < 0 || col >= tilesPerRow)
        return false;
    if (occupancy[row][col] != 0)
        return false;

    std::vector<std::pair<int, int>> footprint = {{row, col}};
    if (violatesAboveRestrictions(footprint))
        return false;

    outBlock.row = row;
    outBlock.col = col;
    return true;
}

bool Editor::TryPlaceCannon(int dropRow, int col, PlacedBlock &outBlock) const
{
    if (col < 0 || col >= tilesPerRow)
        return false;

    const int targetRow = dropRow - dragAnchor;
    if (targetRow < 0 || targetRow + 2 >= tilesPerCol)
        return false;

    if (occupancy[targetRow][col] != 0 ||
        occupancy[targetRow + 1][col] != 0 ||
        occupancy[targetRow + 2][col] != 0)
    {
        return false;
    }

    std::vector<std::pair<int, int>> footprint = {
        {targetRow, col},
        {targetRow + 1, col},
        {targetRow + 2, col}};

    if (violatesAboveRestrictions(footprint))
        return false;

    if (!noBlocksAboveInRestrictedColumns(targetRow + 2, col, col))
        return false;

    outBlock.row = targetRow + 1;
    outBlock.col = col;
    outBlock.anchor = dragAnchor;
    return true;
}

bool Editor::TryPlaceBumper(const glm::vec2 &dropOrigin, PlacedBlock &outBlock) const
{
    const glm::vec2 squareCenterPoint(dropOrigin.x, dropOrigin.y - blockSize * 0.5f);
    int row = 0;
    int col = 0;
    if (!getTileAtPosition(squareCenterPoint, row, col))
        return false;

    if (row < 0 || row + 1 >= tilesPerCol || col <= 0 || col + 1 >= tilesPerRow)
        return false;

    if (occupancy[row][col] != 0 ||
        occupancy[row + 1][col] != 0 ||
        occupancy[row + 1][col - 1] != 0 ||
        occupancy[row + 1][col + 1] != 0)
    {
        return false;
    }

    std::vector<std::pair<int, int>> footprint = {
        {row, col},
        {row + 1, col},
        {row + 1, col - 1},
        {row + 1, col + 1}};

    if (violatesAboveRestrictions(footprint))
        return false;

    if (!noBlocksAboveInRestrictedColumns(row + 1, col - 1, col + 1))
        return false;

    outBlock.row = row;
    outBlock.col = col;
    outBlock.anchor = dragAnchor;
    return true;
}

void Editor::CommitBlockPlacement(const PlacedBlock &block)
{
    placedBlocks.push_back(block);
    updateOccupancyForBlock(placedBlocks.back(), 1);
}

bool Editor::RemoveBlockAtPosition(const glm::vec2 &scenePos)
{
    int tileRow = -1;
    int tileCol = -1;
    const bool hitTile = getTileAtPosition(scenePos, tileRow, tileCol);

    for (int i = static_cast<int>(placedBlocks.size()) - 1; i >= 0; --i)
    {
        if (HitTestBlock(placedBlocks[i], scenePos, hitTile ? tileRow : -1, hitTile ? tileCol : -1))
        {
            updateOccupancyForBlock(placedBlocks[i], 0);
            placedBlocks.erase(placedBlocks.begin() + i);
            statusRemaining = std::min(StatusCount, statusRemaining + 1);
            return true;
        }
    }
    return false;
}

bool Editor::HitTestBlock(const PlacedBlock &block, const glm::vec2 &scenePos, int tileRow, int tileCol) const
{
    switch (block.type)
    {
    case BlockType::BUMPER:
    {
        const Square &baseTile = tiles[block.row][block.col];
        const bool inSquare = (scenePos.x >= baseTile.downLeft.x && scenePos.x <= baseTile.upRight.x &&
                               scenePos.y >= baseTile.downLeft.y && scenePos.y <= baseTile.upRight.y);
        if (inSquare)
            return true;

        const glm::vec2 squareCenter = getTileCenter(block.row, block.col);
        const float centerX = squareCenter.x;
        const float centerY = squareCenter.y + blockSize * 0.5f;
        if (scenePos.y >= centerY)
        {
            const float ovalRadiusX = 1.5f * blockSize;
            const float ovalRadiusY = blockSize;
            const float dx = (scenePos.x - centerX) / ovalRadiusX;
            const float dy = (scenePos.y - centerY) / ovalRadiusY;
            if (dx * dx + dy * dy <= 1.0f)
                return true;
        }
        return false;
    }
    case BlockType::SOLID:
        return tileRow == block.row && tileCol == block.col;
    case BlockType::CANNON:
        return tileCol == block.col &&
               (tileRow == block.row || tileRow == block.row - 1 || tileRow == block.row + 1);
    default:
        return false;
    }
}

void Editor::ResetDragState()
{
    draggedBlock = NONE;
    dragPosition = glm::vec2(0.0f);
    dragOffset = glm::vec2(0.0f);
    dragAnchor = 0;
}
