#pragma once

#include "lab_m1/Arcade-Machine/Editor.hpp"
#include "components/simple_scene.h"
#include "components/text_renderer.h"

namespace m1
{

    class Pong : public gfxc::SimpleScene
    {
    public:
        Pong() = default;
        ~Pong()
        {
            delete textRenderer;
            textRenderer = nullptr;
        }

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

        // Ship layout description
        enum ShipBlockType
        {
            SOLID,
            CANNON,
            BUMPER
        };
        struct ShipBlock
        {
            ShipBlockType type;
            int row;
            int col;
        };

        /**
         * @brief Inject the ship layout from the Editor. blockSize is the Editor's blockSize in logical units.
         *        Assumes the caller provides either an empty layout (no ship) or valid, normalized blocks.
         * @param blocks Vector of ShipBlock describing the ship.
         * @param blockSize Size of a block in logical units (Editor space).
         */
        void SetShipLayout(const std::vector<ShipBlock> &blocks, float blockSize);

    protected:
        // Base logical resolution (same as Editor)
        WindowProperties wp;
        glm::ivec2 baseResolution = wp.resolution;

        // Board
        float margin = 25.f; // space for scores
        glm::vec3 boardOffset = glm::vec3(margin, margin, 0);
        glm::vec2 boardSize = glm::vec2(baseResolution.x - 2 * boardOffset.x, baseResolution.y - 3 * boardOffset.y - margin);

        // Players
        glm::vec2 playerSize = glm::vec2(baseResolution.x / 50, boardSize.y / 5.f);

        // Ball
        float ballSize = fmin(baseResolution.x, baseResolution.y) / 72;
        float ballSpeed = 400.0f;     // units per second
        float maxBallSpeed = 4000.0f; // cap for ballSpeed regardless of ball
        float speedupCooldownInterval = 0.2f;
        struct Ball
        {
            glm::vec2 pos;
            glm::vec2 dir;
            float launchGrace;
            bool launchedFromLeft;
            float speedupCooldown = 0.0f;
        };
        std::vector<Ball> balls;
        float launchGraceDuration = 0.15f;
        int nextServeDirection = 1; // 1 -> towards player 2, -1 -> towards player 1

        // Initial positions and colors
        glm::vec3 player1Offset = glm::vec3(boardOffset.x + margin, boardOffset.y + (boardSize.y - playerSize.y) / 2.0f, 0);
        glm::vec3 player2Offset = glm::vec3(boardOffset.x + boardSize.x - margin - playerSize.x, boardOffset.y + (boardSize.y - playerSize.y) / 2.0f, 0);
        glm::vec3 player1Color = glm::vec3(0, 0.4, 0.8);
        glm::vec3 player2Color = glm::vec3(0.8, 0.8, 0.4);

        // Movement
        float paddleSpeed = 400; // units per second in logical space

        // Scores
        int numberOfRounds = 3;
        int scoreP1 = 0;
        int scoreP2 = 0;

        // End-game message config/state
        float endMessageDuration = 2.0f; // seconds to show final message
        int winningPlayer = 0;           // 1 or 2
        float gameOverTimer = 0.0f;

        // Ship rendering data injected from Editor
        std::vector<ShipBlock> shipLayout;
        float shipBlockSize = 0.0f; // Editor block size in logical units
        int shipMinCol = 0, shipMinRow = 0, shipWidthTiles = 0, shipHeightTiles = 0;
        float shipWidthPx = 0.0f, shipHeightPx = 0.0f;
        // Hard cap for rendered ship thickness after scaling, expressed in scene pixels
        float maxDisplayedShipWidthPx = 400.0f;

        // Colors for ship components (match Editor)
        glm::vec3 solidColor = glm::vec3(0.6f, 0.6f, 0.6f);
        glm::vec3 cannonDarkColor = glm::vec3(0.1f, 0.1f, 0.1f);
        glm::vec3 cannonLightColor = glm::vec3(0.8f, 0.8f, 0.8f);
        glm::vec3 bumperColor = glm::vec3(0.6f, 0, 0.6f);

        float tintStrength = 0.5f;
        float bumperSpeedFactor = 1.15f;

        // Text rendering (new)
        gfxc::TextRenderer *textRenderer = nullptr;
        std::string fontPath = "Hack-Bold.ttf"; // can be changed later
        float uiTextScale = 1.0f;               // dynamic text scale computed from window height

        /**
         * @brief Helper to render the injected ship with a given base model; isLeft selects which tinted meshes to use
         * @param baseModel Base model matrix for ship placement.
         * @param isLeft True if rendering the left ship, false for right ship.
         */
        void RenderShip(const glm::mat3 &baseModel, bool isLeftShip);

        /**
         * @brief Collision using ship geometry (union of tiles): circle vs transformed tiles; sets hitBumper if bumper was touched
         * @param ballCenter Center of the ball in world coordinates.
         * @param ballRadius Radius of the ball.
         * @param baseModel Base model matrix for ship placement.
         * @param uniformScale Uniform scale factor applied to the ship.
         * @param hitBumper Output flag set to true if a bumper was hit.
         * @return true if a collision was detected.
         */
        bool CollideBallWithShip(const glm::vec2 &ballCenter, float ballRadius, const glm::mat3 &baseModel, float uniformScale, bool &hitBumper);

        /**
         * @brief Compute uniform scale factor for ship to fit in board height, and scaled paddle dimensions.
         * @param uniformScale Output uniform scale factor.
         * @param paddleHeightScaled Output scaled paddle height.
         * @param paddleThicknessScaled Output scaled paddle thickness.
         * @return true if ship is present and scale was computed.
         */
        bool ComputeShipScale(float &uniformScale, float &paddleHeightScaled, float &paddleThicknessScaled) const;

        /**
         * @brief Spawn balls at start of a round: from each cannon on both ships if present; otherwise center serve to the right
         */
        void SpawnRoundBalls();

        /**
         * @brief Helper: draw score via TextRenderer
         */
        void DrawScoreText();

        /**
         * @brief Measure the width of the given text at the specified scale.
         * @param text Text to measure.
         * @param scale Scale factor.
         * @return Width of the text in pixels.
         */
        float MeasureTextWidth(const std::string &text, float scale) const;

        /**
         * @brief Accumulate the maximum bearing and descent values for the given text.
         * @param text Text to measure.
         * @param maxBearing Output maximum bearing value.
         * @param maxDescent Output maximum descent value.
         */
        void AccumulateTextMetrics(const std::string &text, float &maxBearing, float &maxDescent) const;

        /**
         * @brief Draw end-of-game message indicating the winner.
         */
        void DrawEndMessage();

        /**
         * @brief Update the UI text scale based on the current window resolution.
         * @param resolution Current window resolution.
         */
        void UpdateUITextScale(const glm::ivec2 &resolution);

        /**
         * @brief Reset cached ship metrics after layout change.
         */
        void ResetShipMetrics();

        /**
         * @brief Update cached ship extents based on current layout.
         * @param row Row index of the block.
         * @param col Column index of the block.
         * @param minRow Output minimum row index.
         * @param maxRow Output maximum row index.
         * @param minCol Output minimum column index.
         * @param maxCol Output maximum column index.
         */
        void UpdateCellExtents(int row, int col, int &minRow, int &maxRow, int &minCol, int &maxCol) const;

        /**
         * @brief Update cached ship extents based on a ShipBlock.
         * @param block ShipBlock to consider.
         * @param minRow Output minimum row index.
         * @param maxRow Output maximum row index.
         * @param minCol Output minimum column index.
         * @param maxCol Output maximum column index.
         */
        void UpdateExtentsForBlock(const ShipBlock &block, int &minRow, int &maxRow, int &minCol, int &maxCol) const;

        /**
         * @brief Ensure base and tinted meshes for ship components are available.
         * @param blockSize Size of a block in logical units.
         */
        void EnsureBaseMeshes(float blockSize);

        /**
         * @brief Ensure tinted meshes for ship components are available.
         * @param suffix Suffix to append to base mesh names for tinted variants.
         * @param tintColor Color used for tinting.
         * @param blockSize Size of a block in logical units.
         */
        void EnsureTintedMeshes(const std::string &suffix, const glm::vec3 &tintColor, float blockSize);

        /**
         * @brief Spawn balls from all cannons on a ship.
         * @param model Model matrix for ship placement.
         * @param leftSide True if spawning from left ship, false for right ship.
         */
        void SpawnBallsFromCannons(const glm::mat3 &model, bool leftSide);

        /**
         * @brief Handle collision of a ball with a ship's paddle.
         * @param ball Ball to check and update.
         * @param model Model matrix for ship placement.
         * @param uniformScale Uniform scale factor applied to the ship.
         * @param paddleCenterY Y coordinate of the paddle center in world space.
         * @param leftSide True if handling left ship, false for right ship.
         */
        void HandlePaddleCollision(Ball &ball, const glm::mat3 &model, float uniformScale, float paddleCenterY, bool leftSide, float paddleHeight);

        /**
         * @brief Update ball position and handle collisions.
         * @param ball Ball to update.
         * @param deltaTimeSeconds Time elapsed since last update in seconds.
         */
        void UpdateBall(Ball &ball, float deltaTimeSeconds);

        /**
         * @brief Check and update scores based on ball positions.
         */
        void CheckScore();

        /**
         * @brief Check if a circle intersects with a rectangle.
         * @param center Center of the circle.
         * @param radius Radius of the circle.
         * @param rx0 Left X coordinate of the rectangle.
         * @param ry0 Bottom Y coordinate of the rectangle.
         * @param rx1 Right X coordinate of the rectangle.
         * @param ry1 Top Y coordinate of the rectangle.
         * @return true if the circle intersects with the rectangle.
         */
        bool CircleIntersectsRect(const glm::vec2 &center, float radius, float rx0, float ry0, float rx1, float ry1) const;

        /**
         * @brief Check if a circle hits the cap of a bumper.
         * @param center Center of the circle.
         * @param radius Radius of the circle.
         * @param ex X coordinate of the bumper center.
         * @param ey Y coordinate of the bumper center.
         * @param rx X coordinate of the rectangle base of the bumper.
         * @param ry Y coordinate of the rectangle base of the bumper.
         * @return true if the circle hits the bumper cap.
         */
        bool CircleHitsBumperCap(const glm::vec2 &center, float radius, float ex, float ey, float rx, float ry) const;
    };
}
