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
        glm::vec3 savedCamPos;
        glm::vec3 savedCamForward;
        glm::vec3 savedCamUp;
        float savedDistanceToTarget = 2.0f;
        glm::vec3 centeredOffset; // offset from locomotive to camera when centered mode is active
        // Minimap settings
        int minimapMargin = 25;         // margin in pixels from screen edges
        float minimapWorldSize = 15.0f; // real-world size shown in minimap (units)

        bool isPerspective = true;
        float fovY = RADIANS(60.0f);
        float zNear = 0.01f;
        float zFar = 200.0f;

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

        // TODO harcoded stations in final positions and rails + stop at intersections and choose with "WAD"

        struct Rail
        {
            glm::vec3 startPosition;
            glm::vec3 endPosition;
            glm::vec3 dir;
            float length = 1.0f;
            std::vector<Rail *> nextRails;
        };

        struct Train
        {
            Rail *rail;
            float progress; // [0, 1] position along the rail
            float speed;    // units per second
            glm::vec3 position;
            glm::vec3 direction; // unit vector
            float yaw = 0.0f;    // orientation around Y
            int numWagons;
            bool waitingAtIntersection = false;
        } train;

        // Symbols above main station (fade to red)
        // --- MODIFICARE: Mecanica de Livrare (Delivery) ---
        float maxDeliveryTime = 30.0f;               // Timpul maxim pentru a ajunge la gara
        float deliveryTimer = 0.0f;                  // Cronometru activ
        bool isDelivering = false;                   // False = colectam, True = ducem la gara
        
        std::vector<int> symbolTypes;                // 0 = cube, 1 = pyramid, 2 = sphere
        std::vector<glm::vec3> symbolOriginalColors; // original colors
        bool symbolsInitialized = false;             // flag to generate random types once

        // Symbol collection mechanics
        static constexpr float kStationRadius = 4.0f;  // detection radius for stations (stations are offset from rails)
        static constexpr float kSymbolWaitTime = 0.5f; // seconds to wait per symbol
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
    };
} // namespace m1
