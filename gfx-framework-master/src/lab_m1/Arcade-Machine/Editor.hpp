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
		Pong* pong = nullptr; // instantiate only when entering pong mode to avoid its camera input in editor
		bool inPongMode = false;

        // circle props (in logical coordinates)
        float circleRadius = 25.f;
        glm::vec2 circleCenter = glm::vec2(baseResolution.x - circleRadius - 2 * margin, baseResolution.y - circleRadius - margin);
		glm::vec3 circleColor = glm::vec3(0, 0.5, 0);
        glm::vec3 readyCircleColor;

        struct Square {
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
        glm::vec3 tileBoardColor = glm::vec3(1, 1, 1);

		// board props
        float gridWidth = tilesPerRow * tileSize + (tilesPerRow - 1) * 2 * tileSpacing;
        float gridHeight = tilesPerCol * tileSize + (tilesPerCol - 1) * 2 * tileSpacing;
        float boardW = gridWidth + 2 * margin;
        float boardH = gridHeight + 2 * margin;
        float boardX = baseResolution.x - boardW - margin; // margin la dreapta si jos
        float boardY = margin;

		// status props
		int StatusCount = 10;
		glm::vec3 statusColor = circleColor;
        float statusSize = 2 * circleRadius;
		int statusSpacing = 10;
		std::vector<Square> statusParts = std::vector<Square>(StatusCount);
        int statusRemaining = StatusCount; // how many status squares are still available

        // part grid props
        int partCount = 3;
        glm::vec2 partSize = glm::vec2(215, (baseResolution.y - 2 * margin) / partCount);

		float blockSize = tileSize + 2 * tileSpacing;

		// Colors for different block types
		glm::vec3 solidColor = glm::vec3(0.6, 0.6, 0.6);
		glm::vec3 cannonDarkColor = glm::vec3(0.1, 0.1, 0.1);
		glm::vec3 cannonLightColor = glm::vec3(0.8, 0.8, 0.8);
		glm::vec3 bumperColor = glm::vec3(0.6, 0, 0.6);

        // Drag and drop state
        enum BlockType { NONE, SOLID, CANNON, BUMPER };
        BlockType draggedBlock = NONE;
        glm::vec2 dragPosition = glm::vec2(0, 0);
        glm::vec2 dragOffset = glm::vec2(0, 0);
        // Anchor for drag: for CANNON -> 0=bottom,1=middle,2=top; for BUMPER -> 0=square,1=semioval; for SOLID ignored
        int dragAnchor = 0;

        // Placed blocks on grid
        struct PlacedBlock {
            BlockType type;
            int row;
            int col;
            int anchor; // see dragAnchor semantics
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
         * @brief Checks whether the Ready circle can be clicked and if the mouse position is inside it.
         *
         * The circle is considered “ready” only when its color is green (0, 0.5, 0).
         * If this condition is met and the cursor is inside the circle, the function returns true.
         *
         * @param mouseX The X coordinate of the mouse in pixels (window space).
         * @param mouseY The Y coordinate of the mouse in pixels (window space).
         * @return true if the circle can be clicked and the mouse is inside it; otherwise false.
         */
        bool checkReady(int mouseX, int mouseY);

        /**
         * @brief Convertește o poziție a mouse-ului din spațiul ferestrei în coordonatele scenei logice.
         *
         * Mapează intervalul [0..rezoluție_fereastră] la [0..baseResolution] și inversează axa Y
         * (în fereastră Y=0 este sus, în scenă Y=0 este jos).
         *
         * @param mouseX Coordonata X în pixeli a mouse-ului (față de colțul stânga-sus).
         * @param mouseY Coordonata Y în pixeli a mouse-ului (față de colțul stânga-sus).
         * @return glm::vec2 Poziția în coordonate logice ale scenei.
         */
        glm::vec2 screenToScene(int mouseX, int mouseY);

        /**
         * @brief Determină tipul de bloc selectabil din panoul stâng în funcție de poziția din scenă.
         *
         * Verifică hitbox-urile pentru cele trei blocuri demonstrative (SOLID, CANNON, BUMPER)
         * afișate în panoul din stânga și întoarce tipul blocului peste care se află cursorul.
         *
         * @param scenePos Poziția cursorului în coordonatele scenei.
         * @return BlockType Tipul blocului detectat; NONE dacă nu s-a intersectat niciun bloc.
         */
        BlockType getBlockAtPosition(glm::vec2 scenePos);

        /**
         * @brief Identifică tile-ul din grilă care conține un anumit punct din scenă.
         *
         * Parcurge grila și verifică limitele fiecărui tile.
         * La succes, întoarce true și setează parametrii de ieșire pentru linie și coloană.
         *
         * @param scenePos Poziția în scena logică de testat.
         * @param row [out] Indexul liniei pentru tile-ul găsit.
         * @param col [out] Indexul coloanei pentru tile-ul găsit.
         * @return true dacă poziția se află în interiorul unui tile; altfel false.
         */
        bool getTileAtPosition(glm::vec2 scenePos, int& row, int& col);

        /**
         * @brief Calculează centrul unui tile din grilă.
         *
         * Utilizează colțul stânga-jos al tile-ului și dimensiunea `tileSize` pentru a determina
         * coordonatele centrului. Este punctul de referință folosit la randarea și plasarea blocurilor.
         *
         * @param row Indexul liniei tile-ului.
         * @param col Indexul coloanei tile-ului.
         * @return glm::vec2 Coordonatele centrului tile-ului.
         */
        glm::vec2 getTileCenter(int row, int col);

        /**
         * @brief Caută un bloc deja plasat exact pe tile-ul (row, col).
         *
         * Această funcție este utilă pentru a preveni suprapunerea blocurilor în același tile.
         *
         * @param row Linia tile-ului.
         * @param col Coloana tile-ului.
         * @return int Indexul în vectorul `placedBlocks` dacă a fost găsit; -1 dacă nu există.
         */
        int findPlacedBlockAt(int row, int col);

        /**
         * @brief Efectuează hit-testing pe geometria blocurilor plasate pentru a găsi un obiect sub cursor.
         *
         * Testează după formele reale:
         * - SOLID: pătrat centrat pe tile
         * - CANNON: trei pătrate stivuite (jos/mijloc/sus)
         * - BUMPER: pătratul de bază + semi-ovala superioară
         * Iterează în ordine inversă pentru a prioriza obiectele plasate cel mai recent.
         *
         * @param scenePos Poziția cursorului în coordonatele scenei.
         * @return int Indexul în `placedBlocks` al obiectului lovit; -1 dacă nu există.
         */
        int findPlacedBlockAtPosition(glm::vec2 scenePos);


        int correctPlacement();

        // Expose ship data to Pong
    public:
        std::vector<PlacedBlock> GetPlacedBlocks() const { return placedBlocks; }
        float GetBlockSize() const { return blockSize; }
    };
}
