#pragma once

#include "components/simple_scene.h"
#include <string>
#include <vector>

namespace m1
{
    class Lab1 : public gfxc::SimpleScene
    {
     public:
        Lab1();
        ~Lab1();

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

        // Class variables
        // Clear color components
        float clearColorR, clearColorG, clearColorB;
        // Object transform
        glm::vec3 objectPosition;
        glm::vec3 objectScale;
        // Mesh cycling support
        std::vector<std::string> meshCycle; // list of mesh IDs we will cycle through
        int currentMeshIndex;
        // Movement speed
        float moveSpeed;
    };
}   // namespace m1
