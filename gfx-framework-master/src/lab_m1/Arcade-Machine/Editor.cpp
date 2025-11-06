#include <glm/gtc/matrix_inverse.hpp>
#include "lab_m1/Arcade-Machine/Editor.hpp"
#include "lab_m1/lab3/transform2D.h"
#include "lab_m1/lab3/object2D.h"
#include <queue>

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

    Mesh* circle = object2D::CreateCircle("circle", glm::vec3(0, 0, 0), circleRadius, glm::vec3(0.5, 0, 0), true);
    AddMeshToList(circle);

	Mesh* tile = object2D::CreateSquare("tile", glm::vec3(0, 0, 0), tileSize, tileColor, true);
	AddMeshToList(tile);
    Mesh* part = object2D::CreateSquare("part", glm::vec3(0, 0, 0), tileSize, glm::vec3(1, 0, 0), false);
    AddMeshToList(part);

    Mesh* tileBoard = object2D::CreateRectangle("tileBoard", glm::vec3(0, 0, 0), boardW, boardH, tileBoardColor, false);
    AddMeshToList(tileBoard);

    Mesh* status = object2D::CreateSquare("status", glm::vec3(0, 0, 0), statusSize, statusColor, true);
    AddMeshToList(status);

    Mesh* solid = object2D::CreateSquare("solid", glm::vec3(0, 0, 0), blockSize, solidColor, true);
    AddMeshToList(solid);
	Mesh* cannon = object2D::CreateCannon("cannon", glm::vec3(0, 0, 0), blockSize, cannonDarkColor, cannonLightColor, true);
	AddMeshToList(cannon);
	Mesh* bumper = object2D::CreateBumper("bumper", glm::vec3(0, 0, 0), blockSize * 1.5f, blockSize, bumperColor, cannonLightColor, true);
    AddMeshToList(bumper);
}

void Editor::FrameStart()
{
    if (inPongMode) {
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
    if (inPongMode) {
        pong->Update(deltaTimeSeconds);
        return;
    }

    // Dynamic ready circle color based on placement validity and at least one block placed
    bool valid = correctPlacement();
    readyCircleColor = (valid ? glm::vec3(0, 0.5f, 0) : glm::vec3(0.5f, 0, 0));

    // Cercul in dreapta sus
    glm::mat3 modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(circleCenter.x, circleCenter.y);

    // Render with override color so it reflects current state
    RenderMesh2D(meshes["circle"], modelMatrix, readyCircleColor);

    // Status bar: render only statusRemaining squares, left-to-right visually (we track removal from right in logic)
    float statusStartX = margin + partSize.x + 2 * margin;
	float statusY = circleCenter.y - statusSize / 2; // center vertically with circle
    for (int i = 0; i < statusRemaining; i++) {
        float x = statusStartX + i * (statusSize + 2 * statusSpacing);
        float y = statusY;
        glm::mat3 statusModel = glm::mat3(1);
        statusModel *= transform2D::Translate(x, y);
        RenderMesh2D(meshes["status"], shaders["VertexColor"], statusModel);
    }

	// Left panel parts
    for (int i = 0; i < partCount; i++) {
        float y = margin + i * partSize.y;
        glm::mat3 partModel = glm::mat3(1);
        partModel *= transform2D::Translate(margin, y);
        partModel *= transform2D::Scale(partSize.x / tileSize, partSize.y / tileSize);
        RenderMesh2D(meshes["part"], shaders["VertexColor"], partModel);
    }

    glm::mat3 boardModel = glm::mat3(1);
    boardModel *= transform2D::Translate(boardX, boardY);
    RenderMesh2D(meshes["tileBoard"], shaders["VertexColor"], boardModel);

	// Tile grid
    float startX = boardX + margin;
    float startY = boardY + margin;

    for (int r = 0; r < tilesPerCol; r++) {
        for (int c = 0; c < tilesPerRow; c++) {
            float x = startX + c * (tileSize + 2 * tileSpacing);
            float y = startY + r * (tileSize + 2 * tileSpacing);

            tiles[r][c].downLeft = glm::vec2(x, y);
            tiles[r][c].upRight = glm::vec2(x + tileSize, y + tileSize);
            
            glm::mat3 tileModel = glm::mat3(1);
            tileModel *= transform2D::Translate(x, y);

            if (r == 5 && c == 9) {
                RenderMesh2D(meshes["tile"], tileModel, mandatoryTileColor);
            } else {
                RenderMesh2D(meshes["tile"], shaders["VertexColor"], tileModel);
            }
        }
    }

    // reuse the same modelMatrix variable declared above
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(solidX, solidY);
    RenderMesh2D(meshes["solid"], shaders["VertexColor"], modelMatrix);

    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(cannonX, cannonY);
    RenderMesh2D(meshes["cannon"], shaders["VertexColor"], modelMatrix);
    
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(bumperX, bumperY);
    RenderMesh2D(meshes["bumper"], shaders["VertexColor"], modelMatrix);

    // Placed blocks
    for (const auto& block : placedBlocks) {
        glm::vec2 center = getTileCenter(block.row, block.col);
        modelMatrix = glm::mat3(1);
        
        switch (block.type) {
            case BlockType::SOLID:
                modelMatrix *= transform2D::Translate(center.x - blockSize / 2, center.y - blockSize / 2);
                RenderMesh2D(meshes["solid"], shaders["VertexColor"], modelMatrix);
                break;
            case BlockType::CANNON: {
                // We stored middle square row in block.row; bottom row is row-1, top is row+1
                float baseY = getTileCenter(block.row, block.col).y - 3 * blockSize / 2; // bottom-left origin
                float baseX = center.x - blockSize / 2;
                modelMatrix *= transform2D::Translate(baseX, baseY);
                RenderMesh2D(meshes["cannon"], shaders["VertexColor"], modelMatrix);
                break;
            }
            case BlockType::BUMPER: {
                // block.row is the row of the SQUARE part
                glm::vec2 squareCenter = getTileCenter(block.row, block.col);
                modelMatrix *= transform2D::Translate(squareCenter.x, squareCenter.y + blockSize / 2);
                RenderMesh2D(meshes["bumper"], shaders["VertexColor"], modelMatrix);
                break;
            }
            default:
                break;
        }
    }

    // Dragged object LAST (honor anchor)
    if (draggedBlock != NONE) {
        modelMatrix = glm::mat3(1);
        // dragPosition is based on the demo object's origin; we leave it as visual only
        modelMatrix *= transform2D::Translate(dragPosition.x, dragPosition.y);
        switch (draggedBlock) {
            case SOLID:
                RenderMesh2D(meshes["solid"], shaders["VertexColor"], modelMatrix);
                break;
            case CANNON:
                RenderMesh2D(meshes["cannon"], shaders["VertexColor"], modelMatrix);
                break;
            case BUMPER:
                RenderMesh2D(meshes["bumper"], shaders["VertexColor"], modelMatrix);
                break;
            default:
                break;
        }
    }
}

void Editor::FrameEnd() {
    if (inPongMode) { pong->FrameEnd(); return; }
}

void Editor::OnInputUpdate(float deltaTime, int mods)
{
    if (inPongMode) { pong->OnInputUpdate(deltaTime, mods); return; }
}

void Editor::OnKeyPress(int key, int mods)
{
    if (inPongMode) { pong->OnKeyPress(key, mods); return; }
}

void Editor::OnKeyRelease(int key, int mods)
{
    if (inPongMode) { pong->OnKeyRelease(key, mods); return; }
}

void Editor::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    if (inPongMode) { pong->OnMouseMove(mouseX, mouseY, deltaX, deltaY); return; }

    if (draggedBlock != NONE) {
        glm::vec2 scenePos = screenToScene(mouseX, mouseY);
        dragPosition = scenePos - dragOffset;
    }
}

void Editor::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    if (inPongMode) { pong->OnMouseBtnPress(mouseX, mouseY, button, mods); return; }

    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT)) {
        // Check if clicking on ready circle
        if (checkReady(mouseX, mouseY)) {
            if (!pong) pong = new Pong();
            // Pass constructed ship to pong BEFORE Init to ensure meshes exist when it starts
            std::vector<m1::ShipBlock> ship;
            ship.reserve(placedBlocks.size());
            for (const auto &pb : placedBlocks) {
                m1::ShipBlock sb; sb.row = pb.row; sb.col = pb.col; sb.anchor = pb.anchor;
                sb.type = (pb.type == SOLID ? m1::ShipBlockType::SOLID : (pb.type == CANNON ? m1::ShipBlockType::CANNON : m1::ShipBlockType::BUMPER));
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

        if (block != NONE) {
            draggedBlock = block;
            // Calculate offset from block center to mouse position
            float blockCenterX, blockCenterY;
            switch (block) {
                case SOLID:
                    blockCenterX = solidX;
                    blockCenterY = solidY;
                    dragAnchor = 0; // not used
                    break;
                case CANNON: {
                    blockCenterX = cannonX;
                    blockCenterY = cannonY;
                    float relY = scenePos.y - blockCenterY; // 0..3*blockSize
                    if (relY < blockSize) dragAnchor = 0; // bottom
                    else if (relY < 2 * blockSize) dragAnchor = 1; // middle
                    else dragAnchor = 2; // top
                    break;
                }
                case BUMPER: {
                    blockCenterX = bumperX;
                    blockCenterY = bumperY;
                    dragAnchor = (scenePos.y <= blockCenterY) ? 0 : 1; // 0=square,1=semioval
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
    if (inPongMode) { pong->OnMouseBtnRelease(mouseX, mouseY, button, mods); return; }

    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT)) {
        if (draggedBlock != NONE) {
            // If no status left, block placement
            if (statusRemaining <= 0) {
                draggedBlock = NONE;
                return;
            }

            // Precompute occupancy from existing placed blocks (disallow placing over any occupied tile)
            const int rows = tilesPerCol;
            const int cols = tilesPerRow;
            std::vector<std::vector<int>> occ(rows, std::vector<int>(cols, 0));
            auto mark = [&](int r, int c) {
                if (r < 0 || r >= rows || c < 0 || c >= cols) return;
                occ[r][c] = 1;
            };
            for (const auto &pb : placedBlocks) {
                switch (pb.type) {
                    case SOLID:
                        mark(pb.row, pb.col);
                        break;
                    case CANNON:
                        mark(pb.row - 1, pb.col);
                        mark(pb.row, pb.col);
                        mark(pb.row + 1, pb.col);
                        break;
                    case BUMPER:
                        mark(pb.row, pb.col);
                        mark(pb.row + 1, pb.col);
                        mark(pb.row + 1, pb.col - 1);
                        mark(pb.row + 1, pb.col + 1);
                        break;
                    default:
                        break;
                }
            }

            auto violatesAboveRestrictions = [&](const std::vector<std::pair<int,int>>& cells) -> bool {
                // For each candidate cell, ensure it's not strictly above an existing bumper/cannon in their restricted columns
                for (const auto &pb : placedBlocks) {
                    if (pb.type == BUMPER) {
                        int topRow = pb.row + 1;
                        int colsB[3] = { pb.col - 1, pb.col, pb.col + 1 };
                        for (const auto &cell : cells) {
                            int r = cell.first, c = cell.second;
                            if (r > topRow && (c == colsB[0] || c == colsB[1] || c == colsB[2])) return true;
                        }
                    } else if (pb.type == CANNON) {
                        int topRow = pb.row + 1; // pb.row is middle
                        for (const auto &cell : cells) {
                            int r = cell.first, c = cell.second;
                            if (r > topRow && c == pb.col) return true;
                        }
                    }
                }
                return false;
            };

            auto noBlocksAboveInRestrictedColumns = [&](int topRow, int colMin, int colMax) -> bool {
                // Ensure no occupied cells strictly above topRow in [colMin..colMax]
                for (int c = colMin; c <= colMax; ++c) {
                    if (c < 0 || c >= cols) continue;
                    for (int r = topRow + 1; r < rows; ++r) {
                        if (occ[r][c]) return false;
                    }
                }
                return true;
            };

            // Block was being dragged
            glm::vec2 scenePos = screenToScene(mouseX, mouseY);
            int row, col;
            
            bool placed = false;
            PlacedBlock newBlock; newBlock.type = draggedBlock; newBlock.anchor = dragAnchor;

            auto cellFree = [&](int r, int c) -> bool {
                if (r < 0 || r >= rows || c < 0 || c >= cols) return false;
                return occ[r][c] == 0;
            };

            if (draggedBlock == BUMPER) {
                // Use the dragged mesh origin (base of semi-oval) to derive the square center
                glm::vec2 dropOrigin = scenePos - dragOffset; // current mesh origin while dragging
                glm::vec2 squareCenterPoint(dropOrigin.x, dropOrigin.y - blockSize / 2.0f);
                if (getTileAtPosition(squareCenterPoint, row, col)) {
                    // Candidate cells for bumper
                    std::vector<std::pair<int,int>> cells;
                    cells.push_back(std::make_pair(row, col));
                    cells.push_back(std::make_pair(row + 1, col));
                    cells.push_back(std::make_pair(row + 1, col - 1));
                    cells.push_back(std::make_pair(row + 1, col + 1));

                    // Check all bumper tiles are in-bounds and free
                    if (cellFree(row, col) && cellFree(row + 1, col) && cellFree(row + 1, col - 1) && cellFree(row + 1, col + 1)) {
                        // Check restrictions vs existing bumpers/cannons below
                        if (!violatesAboveRestrictions(cells)) {
                            // Also ensure no existing blocks strictly above new bumper in its 3 columns
                            int topRow = row + 1;
                            if (noBlocksAboveInRestrictedColumns(topRow, col - 1, col + 1)) {
                                newBlock.row = row;
                                newBlock.col = col;
                                placedBlocks.push_back(newBlock);
                                placed = true;
                            }
                        }
                    }
                }
            } else if (getTileAtPosition(scenePos, row, col)) {
                if (draggedBlock == CANNON) {
                    int targetRow = row - dragAnchor; // bottom anchors at row, middle at row-1, top at row-2
                    // Candidate cells for cannon
                    int cBottom = targetRow;
                    int cMid = targetRow + 1;
                    int cTop = targetRow + 2;
                    std::vector<std::pair<int,int>> cells;
                    cells.push_back(std::make_pair(cBottom, col));
                    cells.push_back(std::make_pair(cMid, col));
                    cells.push_back(std::make_pair(cTop, col));

                    // Check bounds and occupancy for all 3 tiles
                    if (targetRow >= 0 && targetRow + 2 < tilesPerCol &&
                        cellFree(targetRow, col) && cellFree(targetRow + 1, col) && cellFree(targetRow + 2, col)) {
                        // Check restrictions vs existing bumpers/cannons below
                        if (!violatesAboveRestrictions(cells)) {
                            // Ensure no existing blocks strictly above new cannon in its column
                            if (noBlocksAboveInRestrictedColumns(targetRow + 2, col, col)) {
                                newBlock.row = targetRow + 1; // store the middle square row
                                newBlock.col = col;
                                placedBlocks.push_back(newBlock);
                                placed = true;
                            }
                        }
                    }
                } else if (draggedBlock == SOLID) {
                    // Candidate cells for solid
                    std::vector<std::pair<int,int>> cells;
                    cells.push_back(std::make_pair(row, col));

                    if (cellFree(row, col) && !violatesAboveRestrictions(cells)) {
                        newBlock.row = row;
                        newBlock.col = col;
                        placedBlocks.push_back(newBlock);
                        placed = true;
                    }
                }
            }

            if (placed) {
                statusRemaining = std::max(0, statusRemaining - 1);
            }
            
            draggedBlock = NONE;
        }
    }
    
    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_RIGHT)) {

        // Right click to remove placed block and restore one status, if any missing
        glm::vec2 scenePos = screenToScene(mouseX, mouseY);

        // Prefer geometry hit test so any pixel of the object works
        int blockIdx = findPlacedBlockAtPosition(scenePos);
        if (blockIdx == -1) {

            // Fallback to tile-based check if not hitting geometry
            int row, col;
            if (getTileAtPosition(scenePos, row, col)) {
                blockIdx = findPlacedBlockAt(row, col);
            }
        }
        if (blockIdx != -1) {
            placedBlocks.erase(placedBlocks.begin() + blockIdx);

            // restore one status square (no overflow)
            statusRemaining = std::min(StatusCount, statusRemaining + 1);
        }
    }
}

void Editor::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    if (inPongMode) { pong->OnMouseScroll(mouseX, mouseY, offsetX, offsetY); return; }
}

void Editor::OnWindowResize(int width, int height)
{
    
}

bool Editor::checkReady(int mouseX, int mouseY)
{
    // Ready only when placement is valid (green circle)
    if (readyCircleColor !=  glm::vec3(0,0.5,0)) return false;

    glm::vec2 scenePos = screenToScene(mouseX, mouseY);
	return glm::length(scenePos - circleCenter) <= circleRadius;
}

glm::vec2 Editor::screenToScene(int mouseX, int mouseY)
{
    // Map window-space to logical scene-space using current viewport size and base resolution
    glm::ivec2 res = window->GetResolution();
    if (res.x <= 0 || res.y <= 0) return glm::vec2(0, 0);
    
    float sceneX = (static_cast<float>(mouseX) / static_cast<float>(res.x)) * static_cast<float>(baseResolution.x);
    // Invert Y coordinate because window Y=0 is top, scene Y=0 is bottom
    float sceneY = (static_cast<float>(res.y - mouseY) / static_cast<float>(res.y)) * static_cast<float>(baseResolution.y);
    
    return glm::vec2(sceneX, sceneY);
}

Editor::BlockType Editor::getBlockAtPosition(glm::vec2 scenePos)
{
    // Check solid block (first part) - standard square
    float solidX = margin + partSize.x / 2 - blockSize / 2;
    float solidY = margin + partSize.y / 2 - blockSize / 2;
    if (scenePos.x >= solidX && scenePos.x <= solidX + blockSize &&
        scenePos.y >= solidY && scenePos.y <= solidY + blockSize) {
        return SOLID;
    }
    
    // Check cannon block (second part) - 3 squares tall (total height = 3 * blockSize)
    float cannonX = margin + partSize.x / 2 - blockSize / 2;
    float cannonY = margin + partSize.y + partSize.y / 2 - 3 * blockSize / 2;
    if (scenePos.x >= cannonX && scenePos.x <= cannonX + blockSize &&
        scenePos.y >= cannonY && scenePos.y <= cannonY + 3 * blockSize) {
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
        scenePos.y >= squareBottom && scenePos.y <= squareTop) {
        return BUMPER;
    }
    
    // Check semioval part (top part of bumper) - elliptical top half
    // Only check if y >= bumperCenterY (top half)
    if (scenePos.y >= bumperCenterY) {
        float ovalRadiusX = 1.5f * blockSize;
        float ovalRadiusY = blockSize;
        
        // Normalized distance in ellipse coordinates
        float dx = (scenePos.x - bumperCenterX) / ovalRadiusX;
        float dy = (scenePos.y - bumperCenterY) / ovalRadiusY;
        
        // Check if point is inside the ellipse
        if (dx * dx + dy * dy <= 1.0f) {
            return BUMPER;
        }
    }
    
    return NONE;
}

bool Editor::getTileAtPosition(glm::vec2 scenePos, int& row, int& col)
{
    // Check if position is within the tile grid bounds
    for (int r = 0; r < tilesPerCol; r++) {
        for (int c = 0; c < tilesPerRow; c++) {
            if (scenePos.x >= tiles[r][c].downLeft.x && scenePos.x <= tiles[r][c].upRight.x &&
                scenePos.y >= tiles[r][c].downLeft.y && scenePos.y <= tiles[r][c].upRight.y) {
                row = r;
                col = c;
                return true;
            }
        }
    }
    return false;
}

glm::vec2 Editor::getTileCenter(int row, int col)
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

int Editor::findPlacedBlockAt(int row, int col)
{
    for (int i = 0; i < placedBlocks.size(); i++) {
        if (placedBlocks[i].row == row && placedBlocks[i].col == col) {
            return i;
        }
    }
    return -1;
}

int Editor::findPlacedBlockAtPosition(glm::vec2 scenePos)
{
    // iterate in reverse so top-most (last placed) gets priority
    for (int i = (int)placedBlocks.size() - 1; i >= 0; --i) {
        const auto &obj = placedBlocks[i];
        // compute bounding/hit according to type
        if (obj.type == SOLID) {
            // single square centered on tile
            glm::vec2 center = getTileCenter(obj.row, obj.col);
            float left = center.x - blockSize / 2;
            float right = center.x + blockSize / 2;
            float bottom = center.y - blockSize / 2;
            float top = center.y + blockSize / 2;
            if (scenePos.x >= left && scenePos.x <= right && scenePos.y >= bottom && scenePos.y <= top)
                return i;
        } else if (obj.type == CANNON) {
            // 3 stacked squares: middle at obj.row
            glm::vec2 mid = getTileCenter(obj.row, obj.col);
            // compute three squares centers
            glm::vec2 centers[3] = {
                glm::vec2(mid.x, mid.y - blockSize), // bottom
                mid,                                  // middle
                glm::vec2(mid.x, mid.y + blockSize)  // top
            };
            for (int k = 0; k < 3; ++k) {
                float left = centers[k].x - blockSize / 2;
                float right = centers[k].x + blockSize / 2;
                float bottom = centers[k].y - blockSize / 2;
                float top = centers[k].y + blockSize / 2;
                if (scenePos.x >= left && scenePos.x <= right && scenePos.y >= bottom && scenePos.y <= top)
                    return i;
            }
        } else if (obj.type == BUMPER) {
            // square part (always hit-test)
            glm::vec2 squareCenter = getTileCenter(obj.row, obj.col);
            float left = squareCenter.x - blockSize / 2;
            float right = squareCenter.x + blockSize / 2;
            float bottom = squareCenter.y - blockSize / 2;
            float top = squareCenter.y + blockSize / 2;
            if (scenePos.x >= left && scenePos.x <= right && scenePos.y >= bottom && scenePos.y <= top)
                return i;
            // semi-oval part centered above the square
            float ovalCenterX = squareCenter.x;
            float ovalCenterY = squareCenter.y + blockSize / 2;
            float ovalRadiusX = 1.5f * blockSize;
            float ovalRadiusY = blockSize;
            if (scenePos.y >= ovalCenterY) {
                float dx = (scenePos.x - ovalCenterX) / ovalRadiusX;
                float dy = (scenePos.y - ovalCenterY) / ovalRadiusY;
                if (dx * dx + dy * dy <= 1.0f)
                    return i;
            }
        }
    }
    return -1;
}

int Editor::correctPlacement()
{
    // Build occupancy grid of tiles covered by any part of any placed block.
    // Return 1 if there is at least one tile occupied, no overlaps, mandatory center tile is covered,
    // and all occupied tiles form a single 4-connected component.
    const int rows = tilesPerCol;
    const int cols = tilesPerRow;

    std::vector<std::vector<int>> occ(rows, std::vector<int>(cols, 0));
    int total = 0;

    auto addCell = [&](int r, int c) -> bool {
        if (r < 0 || r >= rows || c < 0 || c >= cols) return true; // ignore out-of-bounds silently
        if (occ[r][c]) return false; // overlap detected
        occ[r][c] = 1;
        ++total;
        return true;
    };

    for (const auto &pb : placedBlocks) {
        switch (pb.type) {
            case SOLID: {
                if (!addCell(pb.row, pb.col)) return 0;
                break;
            }
            case CANNON: {
                if (!addCell(pb.row - 1, pb.col)) return 0;
                if (!addCell(pb.row, pb.col)) return 0;
                if (!addCell(pb.row + 1, pb.col)) return 0;
                break;
            }
            case BUMPER: {
                // square tile
                if (!addCell(pb.row, pb.col)) return 0;
                // semi-oval occupies the three tiles above: (row+1, col-1/col/col+1)
                if (!addCell(pb.row + 1, pb.col)) return 0;
                if (!addCell(pb.row + 1, pb.col - 1)) return 0;
                if (!addCell(pb.row + 1, pb.col + 1)) return 0;
                break;
            }
            default:
                break;
        }
    }

    if (total == 0) return 0; // must have at least one block

    // Mandatory center tile must be covered
    int midR = tilesPerCol / 2;  // 11 -> 5
    int midC = tilesPerRow / 2;  // 19 -> 9
    if (midR < 0 || midR >= rows || midC < 0 || midC >= cols) return 0;
    if (!occ[midR][midC]) return 0;

    // BFS over occupied grid to ensure connectivity
    std::queue<std::pair<int,int>> q;
    std::vector<std::vector<int>> vis(rows, std::vector<int>(cols, 0));

    for (int r = 0; r < rows; ++r) {
        bool found = false;
        for (int c = 0; c < cols; ++c) {
            if (occ[r][c]) { q.push(std::make_pair(r,c)); vis[r][c] = 1; found = true; break; }
        }
        if (found) break;
    }

    int visited = 0;
    const int dr[4] = {1,-1,0,0};
    const int dc[4] = {0,0,1,-1};

    while (!q.empty()) {
        std::pair<int,int> cur = q.front(); q.pop();
        int r = cur.first;
        int c = cur.second;
        ++visited;
        for (int k = 0; k < 4; ++k) {
            int nr = r + dr[k], nc = c + dc[k];
            if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) continue;
            if (!occ[nr][nc] || vis[nr][nc]) continue;
            vis[nr][nc] = 1;
            q.push(std::make_pair(nr,nc));
        }
    }

    return visited == total ? 1 : 0;
}