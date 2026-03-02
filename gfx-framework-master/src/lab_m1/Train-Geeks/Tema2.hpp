#pragma once

#include "components/simple_scene.h"
#include "lab_m1/lab3/transform2D.h"
#include "lab_m1/lab4/transform3D.h"
#include "lab_m1/lab5/lab_camera.h"
#include "components/text_renderer.h"
#include <iomanip>
#include <vector>
#include <string>
#include <iostream>

namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
    public:
        Tema2();
        ~Tema2();

        void Init() override;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void RenderMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix) override;
        void RenderRailAt(const glm::vec3 &pos, bool rotate90);

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

    protected:
        implemented::Camera *camera;
        glm::mat4 projectionMatrix;

        // Camera toggle: free vs centered-on-locomotive
        bool cameraCentered = false;
        glm::vec3 centeredOffset; // offset from locomotive to camera when centered mode is active
        // Minimap settings
        int minimapMargin = 25;         // margin in pixels from screen edges

        bool isPerspective = true;
        float fovY = RADIANS(60.0f); // angle (in radians) for perspective projection
        float zNear = 0.01f; // near plane
        float zFar = 200.0f; // far plane / draw distance
        float aspect = 16.0f / 9.0f;
        
        float wagonLength = 2.1f;
        float locomotiveLength = 2.0f;

        // Ending screen
        gfxc::TextRenderer *textRenderer = nullptr;
        std::string fontPath = "Hack-Bold.ttf";
        bool endScreenActive = false;
        float endScreenTime = 2.0f;
        float timer = 0.0f;

        // HUD timer
        float elapsedTime = 0.0f;
        glm::vec3 hudTimerColor = glm::vec3(0.0f, 0.0f, 0.0f);

        struct Rail
        {
            glm::vec3 startPosition;
            glm::vec3 endPosition;
            glm::vec3 direction; // OX or OZ unit vector
            float length = 1.0f;
            std::vector<Rail *> nextRails;
        };
        
        struct Train
        {
            Rail *rail;
            float progress; // [0, 1] position along the rail
            float speed;    // units per second
            glm::vec3 position;
            glm::vec3 direction; // relative
            float yaw = 0.0f;    // orientation in OXZ plane (radians) absolute
            int numWagons;
            bool waitingAtIntersection = false;
        } train;
                
        Rail *g_prevRail = nullptr;
        std::vector<Rail> rails;

        // Symbols above main station (fade to red)
        float maxDeliveryTime = 30.0f;               // Time limit to deliver symbols
        float deliveryTimer = 0.0f;                  // Current delivery timer
        bool isDelivering = false;                   // False = collecting, True = delivering to station
        
        std::vector<int> symbolTypes;                // 0 = cube, 1 = pyramid, 2 = sphere
        std::vector<glm::vec3> symbolOriginalColors; // original colors
        bool symbolsInitialized = false;             // flag to generate random types once

        // Station positions
        glm::vec3 stationPositions[3] = {
            glm::vec3(41.0f, 0.0f, -13.0f), // cubeStation
            glm::vec3(7.5f, 0.0f, 17.5f),   // pyramidStation
            glm::vec3(-23.0f, 0.0f, -13.0f) // sphereStation
        };

        float kStopOffset = 1.5f; // distance kept away from intersection center

        // Symbol collection mechanics
        float kStationRadius = 4.0f;                   // detection radius for stations (stations are offset from rails)
        float kSymbolWaitTime = 0.5f;                  // seconds to wait per symbol
        int symbolsCollected = 0;                      // number of symbols collected
        bool collectingSymbol = false;                 // train is stopped collecting symbols
        float collectTimer = 0.0f;                     // timer for current symbol collection
        int symbolsToCollect = 0;                      // remaining consecutive symbols at this stop
        int currentSymbolCount = 5;                    // number of symbols in current order (increases on success)

        /**
         * @brief Get the Rail Type object
         * @param pos Current position of the train
         * @return std::string Type of rail at the given position
         */
        std::string getRailType(glm::vec3 pos);

        /**
         * @brief Convert a direction vector to a yaw angle in radians
         * @param dir Direction vector
         * @return float Yaw angle in radians
         */
        float DirToYaw(const glm::vec3 &dir);
    };
} // namespace m1
