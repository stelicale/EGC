#pragma once

#include "components/simple_scene.h"


namespace m1
{
    class Lab3 : public gfxc::SimpleScene
    {
    public:
        Lab3();
        ~Lab3();

        void Init() override;

    private:
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
        float cx, cy;
        glm::mat3 modelMatrix;
        float translateX, translateY;
        float scaleX, scaleY;
        float angularStep;

        // Ping-pong scaling controls
        float scaleMin = 0.5f;
        float scaleMax = 2.0f;
        float scaleSpeed = 2.0f;
        int scaleDir = 1; // 1 = growing, -1 = shrinking

        // Independent ping-pong translation for square3
        float translateMin = 0.0f;
        float translateMax = 100.0f;
        float translateSpeedY = 100.0f;
        int moveDirY = 1; // 1 = up, -1 = down

        // TODO(student): If you need any other class variables, define them here.

    };
}   // namespace m1
