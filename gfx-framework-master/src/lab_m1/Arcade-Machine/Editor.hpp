#pragma once

#include "components/simple_scene.h"
#include "lab_m1/Arcade-Machine/Pong.hpp"
#include <vector>

namespace m1
{
    class Editor : public gfxc::SimpleScene
    {
    public:
        Editor() = default;
        ~Editor() = default;

        void Init() override;
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;
        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

    protected:
        // Logical base resolution used for orthographic projection
        glm::ivec2 baseResolution = glm::ivec2(1280, 720);

        float margin = 25.f;

        // Switching state
        Pong *pong = nullptr; // instantiate only when entering pong mode to avoid its camera input in editor
        bool inPongMode = false;

        // circle props (in logical coordinates)
        float circleRadius = 25.f;
        glm::vec2 circleCenter = glm::vec2(baseResolution.x - circleRadius - 2 * margin, baseResolution.y - circleRadius - margin);
        glm::vec3 circleColor = glm::vec3(0, 0.5, 0);
        glm::vec3 readyCircleColor;

        struct Square
        {
            glm::vec2 upRight, downLeft;
        };

        // tile grid props
        float tileSize = 40;
        int tileSpacing = 5;
        int tilesPerRow = 19;
        int tilesPerCol = 11;
        glm::vec3 tileColor = glm::vec3(0.4, 0.4, 0.8);
        glm::vec3 mandatoryTileColor = glm::vec3(0.3, 0.3, 0.6);
        std::vector<std::vector<Square>> tiles = std::vector<std::vector<Square>>(tilesPerCol, std::vector<Square>(tilesPerRow));
        std::vector<std::vector<int>> occupancy = std::vector<std::vector<int>>(tilesPerCol, std::vector<int>(tilesPerRow, 0));
        glm::vec3 tileBoardColor = glm::vec3(1, 1, 1);

        // board props
        float gridWidth = tilesPerRow * tileSize + (tilesPerRow - 1) * 2 * tileSpacing;
        float gridHeight = tilesPerCol * tileSize + (tilesPerCol - 1) * 2 * tileSpacing;
        float boardW = gridWidth + 2 * margin;
        float boardH = gridHeight + 2 * margin;
        float boardX = baseResolution.x - boardW - margin;
        float boardY = margin;

        // status props
        int StatusCount = 10;
        glm::vec3 statusColor = circleColor;
        float statusSize = 2 * circleRadius;
        int statusSpacing = 10;
        std::vector<Square> statusParts = std::vector<Square>(StatusCount);
        int statusRemaining = StatusCount;

        // part grid props
        int partCount = 3;
        glm::vec2 partSize = glm::vec2(215, (baseResolution.y - 2 * margin) / partCount);

        float blockSize = tileSize + 2 * tileSpacing;

        // Colors for different block types
        glm::vec3 solidColor = glm::vec3(0.6f, 0.6f, 0.6f);
        glm::vec3 cannonDarkColor = glm::vec3(0.1f, 0.1f, 0.1f);
        glm::vec3 cannonLightColor = glm::vec3(0.8f, 0.8f, 0.8f);
        glm::vec3 bumperColor = glm::vec3(0.6f, 0.0f, 0.6f);

        // Drag and drop state
        enum BlockType
        {
            NONE,
            SOLID,
            CANNON,
            BUMPER
        };
        BlockType draggedBlock = NONE;
        glm::vec2 dragPosition = glm::vec2(0, 0);
        glm::vec2 dragOffset = glm::vec2(0, 0);
        // Anchor for drag: for CANNON -> 0=bottom,1=middle,2=top; for BUMPER -> 0=square,1=semioval; for SOLID ignored
        int dragAnchor = 0;

        // Placed blocks on grid
        struct PlacedBlock
        {
            BlockType type;
            int row;
            int col;
            int anchor;
        };
        std::vector<PlacedBlock> placedBlocks;

        // Demo positions for left panel blocks
        float solidX = margin + partSize.x / 2 - blockSize / 2;
        float solidY = margin + partSize.y / 2 - blockSize / 2;
        float cannonX = margin + partSize.x / 2 - blockSize / 2;
        float cannonY = margin + partSize.y + partSize.y / 2 - 3 * blockSize / 2;
        float bumperX = margin + partSize.x / 2;
        float bumperY = margin + 2 * partSize.y + partSize.y / 2;

        /**
         * @brief Check if the Ready circle is clickable and the mouse is inside it.
         *
         * The circle becomes clickable only when the current layout is valid (green).
         *
         * @param mouseX Mouse X coordinate in window pixels.
         * @param mouseY Mouse Y coordinate in window pixels.
         * @return true if Ready can be clicked and the cursor is within the circle; false otherwise.
         */
        bool checkReady(int mouseX, int mouseY);

        /**
         * @brief Convert a window-space mouse position to logical scene coordinates.
         *
         * Maps [0..window] to [0..baseResolution] and flips Y so that scene origin is bottom-left.
         *
         * @param mouseX Mouse X coordinate in window pixels.
         * @param mouseY Mouse Y coordinate in window pixels.
         * @return glm::vec2 Scene-space coordinates.
         */
        glm::vec2 screenToScene(int mouseX, int mouseY);

        /**
         * @brief Hit-test the left panel demo blocks to detect which block is under the cursor.
         *
         * Checks the hitboxes for SOLID, CANNON, and BUMPER preview blocks.
         *
         * @param scenePos Cursor position in scene coordinates.
         * @return BlockType The detected block type; NONE if no block is hit.
         */
        BlockType getBlockAtPosition(glm::vec2 scenePos);

        /**
         * @brief Find grid cell (row, col) containing a given scene-space point.
         *
         * Iterates the tile grid bounds and returns the indices of the tile containing the point.
         *
         * @param scenePos Test position in scene coordinates.
         * @param row [out] Row index if found.
         * @param col [out] Column index if found.
         * @return true if the point lies inside a tile; false otherwise.
         */
        bool getTileAtPosition(glm::vec2 scenePos, int &row, int &col);

        /**
         * @brief Compute the visual center of a grid tile at (row, col).
         *
         * @param row Tile row index.
         * @param col Tile column index.
         * @return glm::vec2 Center coordinates of the tile in scene space.
         */
        glm::vec2 getTileCenter(int row, int col);

        /**
         * @brief Find an already placed block exactly located on tile (row, col).
         *
         * @param row Tile row to search.
         * @param col Tile column to search.
         * @return Index in `placedBlocks` if found; -1 otherwise.
         */
        int findPlacedBlockAt(int row, int col);

        /**
         * @brief Validate the current layout and return whether it satisfies all constraints.
         *
         * Builds an occupancy map of tiles covered by placed blocks; returns true only if:
         * - At least one tile is occupied;
         * - There are no overlapping tiles;
         * - The mandatory center tile is covered;
         * - All occupied tiles form a single 4-connected component;
         * - Structural restrictions for CANNON/BUMPER columns are satisfied.
         *
         * @return true if the placement is valid; false otherwise.
         */
        bool correctPlacement();

        /** 
         * @brief Check if any of the specified cells violate placement restrictions from blocks below.
         *
         * Used to ensure that CANNON and BUMPER blocks do not have conflicting blocks directly beneath them.
         *
         * @param cells List of (row, col) pairs to check.
         * @return true if any cell violates restrictions; false otherwise.
         */
        bool violatesAboveRestrictions(const std::vector<std::pair<int, int>> &cells) const;

    /**
     * @brief Check if there are no blocks above the specified columns within a restricted row range (for dragged blocks).
     *
     * @param topRow The top row of the restricted area.
     * @param colMin The minimum column index of the restricted area.
     * @param colMax The maximum column index of the restricted area.
     * @return true if no blocks are above the restricted columns; false otherwise.
     */
    bool noBlocksAboveInRestrictedColumns(int topRow, int colMin, int colMax) const;

    /**
     * @brief Write occupancy values for all tiles covered by a block.
     *
     * @param block The block whose footprint is applied to the occupancy grid.
     * @param value Value written to each covered tile (0 clears, 1 marks occupied).
     */
    void updateOccupancyForBlock(const PlacedBlock &block, int value);

        // Expose ship data to Pong
    public:
        /**
         * @brief Get a copy of all currently placed blocks.
         * @return std::vector<PlacedBlock> The placed blocks list.
         */
        std::vector<PlacedBlock> GetPlacedBlocks() const { return placedBlocks; }
        /**
         * @brief Get the logical block size used to render and place parts on the grid.
         * @return float Block size in scene units.
         */
        float GetBlockSize() const { return blockSize; }
    };
}
