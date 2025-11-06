#pragma once

#include "components/simple_scene.h"
#include <vector>
#include <glm/glm.hpp>

namespace m1
{
    // Ship layout description (same style as Editor blocks, but locally defined here; no new files)
    enum class ShipBlockType
    {
        SOLID = 1,
        CANNON = 2,
        BUMPER = 3
    };
    struct ShipBlock
    {
        ShipBlockType type;
        int row;
        int col;
        int anchor; // kept for completeness
    };

    class Pong : public gfxc::SimpleScene
    {
    public:
        Pong() = default;
        ~Pong() override = default;

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

        // Inject the ship layout from the Editor. blockSize is the Editor's blockSize in logical units.
        void SetShipLayout(const std::vector<ShipBlock> &blocks, float blockSize);

    protected:
        // Base logical resolution (same as Editor)
        glm::vec2 baseResolution = glm::vec2(1280, 720);

        // Board
        float scoreOffset = 50.f; // space for scores
        glm::vec3 boardOffset = glm::vec3(25, 25, 0);
        glm::vec2 boardSize = glm::vec2(baseResolution.x - 2 * boardOffset.x, baseResolution.y - 3 * boardOffset.y - scoreOffset);

        // Players
        glm::vec2 playerSize = glm::vec2(baseResolution.x / 50, boardSize.y / 5.f);

        // Ball
        float ballSize = fmin(baseResolution.x, baseResolution.y) / 72;
        // Single global ballSpeed; multiple balls use the same speed value
        float ballSpeed = 400.0f; // units per second
        // Universal maximum speed applied to the shared ball speed
        float maxBallSpeed = 4000.0f; // cap for ballSpeed regardless of ball
        // Minimum interval between consecutive speedups per ball (seconds)
        float speedupCooldownInterval = 0.2;
        struct Ball
        {
            glm::vec2 pos;
            glm::vec2 dir;
            float launchGrace;
            bool launchedFromLeft;
            // Per-ball cooldown timer to throttle bumper speedups
            float speedupCooldown = 0.0f;
        };
        std::vector<Ball> balls;
        // Duration to ignore collision with launching paddle to keep horizontal launch
        float launchGraceDuration = 0.15f;

        // Initial positions and colors
        glm::vec3 player1Offset = glm::vec3(boardOffset.x + 25, boardOffset.y + (boardSize.y - playerSize.y) / 2.0f, 0);
        glm::vec3 player2Offset = glm::vec3(boardOffset.x + boardSize.x - 25 - playerSize.x, boardOffset.y + (boardSize.y - playerSize.y) / 2.0f, 0);
        glm::vec3 player1Color = glm::vec3(0, 0.4, 0.8);
        glm::vec3 player2Color = glm::vec3(0.8, 0.8, 0.4);

        // Alignment helper: recenter paddles once whenever ship layout changes
        bool paddlesNeedAlignment = true;

        // Movement
        float paddleSpeed = 400; // units per second in logical space

        // Scores
        int numberOfRounds = 3;
        int scoreP1 = 0;
        int scoreP2 = 0;

        // Ship rendering data injected from Editor
        std::vector<ShipBlock> shipLayout;
        float shipBlockSize = 0.0f; // Editor block size in logical units
        // cached bounds in tile units
        int shipMinCol = 0, shipMinRow = 0, shipWidthTiles = 0, shipHeightTiles = 0;
        float shipWidthPx = 0.0f, shipHeightPx = 0.0f;
        bool hasShip = false;

        // Colors for ship components (match Editor)
        glm::vec3 solidColor = glm::vec3(0.6f, 0.6f, 0.6f);
        glm::vec3 cannonDarkColor = glm::vec3(0.1f, 0.1f, 0.1f);
        glm::vec3 cannonLightColor = glm::vec3(0.8f, 0.8f, 0.8f);
        glm::vec3 bumperColor = glm::vec3(0.6f, 0, 0.6f);

        // Single global tint strength [0..1] applied to both ships (0=no tint, 1=full tint)
        float tintStrength = 0.5f;

        // Factor to multiply the ball speed when hitting a bumper
        float bumperSpeedFactor = 1.1f;

        // Helper to render the injected ship with a given base model; isLeft selects which tinted meshes to use
        void RenderShip(const glm::mat3 &baseModel, bool isLeftShip);
        // Collision using ship geometry (union of tiles): circle vs transformed tiles; sets hitBumper if bumper was touched
        bool CollideBallWithShip(const glm::vec2 &ballCenter, float ballRadius, const glm::mat3 &baseModel, float uniformScale, bool &hitBumper);

        // Spawn balls at start of a round: from each cannon on both ships if present; otherwise center serve to the right
        void SpawnRoundBalls();

        // Ensure paddle offsets match the current ship layout before spawning or rendering
        void ApplyPaddleAlignmentIfNeeded();
    };
}
