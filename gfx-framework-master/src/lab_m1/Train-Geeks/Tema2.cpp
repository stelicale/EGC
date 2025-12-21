#include "lab_m1/Train-Geeks/Tema2.hpp"
#include "lab_m1/Train-Geeks/object3D.cpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;

namespace m1
{

    static float DirToYaw(const glm::vec3 &dir)
    {
        // Align yaw to mesh forward: +X -> 0 deg, adjust to avoid 180 flips on turns
        return std::atan2(dir.x, dir.z) - glm::half_pi<float>();
    }

    static constexpr float kStopOffset = 1.5f; // distance kept away from intersection center

    std::string Tema2::getRailType(glm::vec3 pos)
    {
        // apă: mesh water (20x40) translat la x=30 -> x in [20,40], z in [-20,20]
        if (pos.x >= 20 && pos.x <= 40 && pos.z >= -20 && pos.z <= 20)
        {
            return "bridgeRail"; // water area
        }
        // munte: mesh mountain (15x30) translat la x=-15 -> x in [-22.5, -7.5], z in [-15,15]
        if (pos.x >= -22.5f && pos.x <= -7.5f && pos.z >= -15 && pos.z <= 15)
        {
            return "tunnelRail"; // mountain area
        }
        return "basicRail"; // normal rail
    }

    Tema2::Tema2()
    {
    }

    Tema2::~Tema2()
    {
        delete textRenderer;
        textRenderer = nullptr;
    }

    void Tema2::Init()
    {
        // Text renderer for ending screen and HUD
        glm::ivec2 resolution = window ? window->GetResolution() : glm::ivec2(1280, 720);
        delete textRenderer;
        textRenderer = new gfxc::TextRenderer(window->props.selfDir, resolution.x, resolution.y);
        textRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, fontPath), 64);

        camera = new implemented::Camera();
        camera->Set(glm::vec3(0, 3, 7), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

        float aspectInit = 16.0f / 9.0f;
        if (window)
        {
            const auto &res = window->props.resolution;
            if (res.y != 0)
                aspectInit = static_cast<float>(res.x) / static_cast<float>(res.y);
            else if (std::abs(window->props.aspectRatio) > std::numeric_limits<float>::epsilon())
                aspectInit = window->props.aspectRatio;
        }
        if (std::abs(aspectInit) <= std::numeric_limits<float>::epsilon())
            aspectInit = 1.0f;

        projectionMatrix = glm::perspective(fovY, aspectInit, zNear, zFar);

        cameraCentered = false;
        if (camera)
        {
            savedCamPos = camera->position;
            savedCamForward = camera->forward;
            savedCamUp = camera->up;
            savedDistanceToTarget = camera->distanceToTarget;
            glm::vec3 locoPosInit = glm::vec3(0.0f, 0.6f, 0.0f);
            centeredOffset = savedCamPos - locoPosInit;
        }

        // --- MESH CREATION ---
        Mesh *ground = object3D::CreateGround("ground", 100.0f, glm::vec3(0.4f, 0.8f, 0.4f));
        AddMeshToList(ground);

        Mesh *water = object3D::CreateWater("water", 20, 40);
        AddMeshToList(water);

        Mesh *mountain = object3D::CreateMountain("mountain", 15, 30);
        AddMeshToList(mountain);

        Mesh *tree = object3D::CreateTree("tree", glm::vec3(0.55f, 0.27f, 0.07f), glm::vec3(0.0f, 0.5f, 0.0f));
        AddMeshToList(tree);

        Mesh *locomotive = object3D::CreateLocomotive("locomotive", glm::vec3(0.0f, 0.7f, 0.0f), glm::vec3(0.0f, 0.0f, 0.7f), glm::vec3(0.7f, 0.0f, 0.0f));
        AddMeshToList(locomotive);

        Mesh *wagon = object3D::CreateWagon("wagon", glm::vec3(0.0f, 0.7f, 0.0f), glm::vec3(0.7f, 0.0f, 0.0f), glm::vec3(1.0f, 0.8f, 0.0f));
        AddMeshToList(wagon);

        Mesh *sphereStation = object3D::CreateBasicTrainStation("sphereStation", glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(0.4f, 0.2f, 0.1f), glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.7f, 0.5f, 0.3f), "sphere");
        AddMeshToList(sphereStation);

        Mesh *cubeStation = object3D::CreateBasicTrainStation("cubeStation", glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(0.4f, 0.2f, 0.1f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.7f, 0.5f, 0.3f), "cube");
        AddMeshToList(cubeStation);

        Mesh *pyramidStation = object3D::CreateBasicTrainStation("pyramidStation", glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(0.4f, 0.2f, 0.1f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.7f, 0.5f, 0.3f), "pyramid");
        AddMeshToList(pyramidStation);

        Mesh *mainStation = object3D::CreateMainTrainStation("mainStation", glm::vec3(0.8f, 0.7f, 0.6f), glm::vec3(0.6f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f));
        AddMeshToList(mainStation);

        Mesh *basicRail = object3D::CreateNormalRail("basicRail", glm::vec3(0.3f, 0.3f, 0.3f), 10.0f);
        AddMeshToList(basicRail);

        Mesh *tunnelRail = object3D::CreateTunnelRail("tunnelRail", glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(0.2f, 0.2f, 0.25f), 10.0f);
        AddMeshToList(tunnelRail);

        Mesh *bridgeRail = object3D::CreateBridgeRail("bridgeRail", glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(0.5f, 0.4f, 0.3f), 10.0f);
        AddMeshToList(bridgeRail);

        // 1. Cerc Turcuaz (Sfera)
        Mesh *mapCircle = object3D::CreateCircle("map_circle", glm::vec3(0, 1, 1));
        AddMeshToList(mapCircle);

        // 2. Triunghi Verde (Piramida)
        Mesh *mapTriangle = object3D::CreateTriangle("map_triangle", glm::vec3(0, 1, 0));
        AddMeshToList(mapTriangle);

        // 3. Pătrat Albastru (Cub)
        Mesh *mapSquare = object3D::CreateSquare("map_square", glm::vec3(0, 0, 1));
        AddMeshToList(mapSquare);

        // 4. Stea Mov (Gara Centrală)
        Mesh *mapStar = object3D::CreateStar("map_star", glm::vec3(0.6f, 0.0f, 0.8f));
        AddMeshToList(mapStar);

        // 5. Săgeată și Tren
        Mesh *mapArrow = object3D::CreateArrow("map_arrow", glm::vec3(1, 1, 1));
        AddMeshToList(mapArrow);

        Mesh *mapTrain = object3D::CreateSquare("map_train", glm::vec3(1.0f, 1.0f, 0.3f));
        AddMeshToList(mapTrain);

        // 6. Stripe pentru pod (Pătrat Alb) - ADAUGAT PENTRU MINIMAP
        Mesh *mapStripe = object3D::CreateSquare("map_stripe", glm::vec3(1, 1, 1));
        AddMeshToList(mapStripe);

        // --- INTERSECTIONS ---
        glm::vec3 railCol(0.3f, 0.3f, 0.3f);
        glm::vec3 tunnelCol(0.2f, 0.2f, 0.25f);
        glm::vec3 bridgeCol(0.5f, 0.4f, 0.3f);

        AddMeshToList(object3D::CreateNormalRailIntersection("intNormal_-20_-20", railCol, true, false, true, false));
        AddMeshToList(object3D::CreateNormalRailIntersection("intNormal_-20_20", railCol, false, true, true, false));
        AddMeshToList(object3D::CreateBridgeRailIntersection("intBridge_38_-20", railCol, bridgeCol, true, false, false, true));
        AddMeshToList(object3D::CreateBridgeRailIntersection("intBridge_38_20", railCol, bridgeCol, false, true, false, true));
        AddMeshToList(object3D::CreateTunnelRailIntersection("intTunnel_-20_0", railCol, tunnelCol, true, true, true, false));
        AddMeshToList(object3D::CreateBridgeRailIntersection("intBridge_38_0", railCol, bridgeCol, true, true, false, true));

        // Intersectiile centrale
        AddMeshToList(object3D::CreateNormalRailIntersection("intNormal_2.5_-20", railCol, true, false, true, true));
        AddMeshToList(object3D::CreateNormalRailIntersection("intNormal_2.5_20", railCol, false, true, true, true));
        AddMeshToList(object3D::CreateNormalRailIntersection("intNormal_2.5_0", railCol, true, true, true, true));

        // --- RAIL NETWORK BUILD ---
        static std::vector<Rail> rails_local;
        rails_local.clear();
        rails_local.reserve(32);

        auto closeEnough = [](const glm::vec3 &a, const glm::vec3 &b, float eps = 0.01f)
        {
            return glm::length(a - b) <= eps;
        };

        auto makeRail = [&](const glm::vec3 &start, const glm::vec3 &end) -> Rail *
        {
            rails_local.push_back({});
            Rail &rail = rails_local.back();
            rail.startPosition = start;
            rail.endPosition = end;
            glm::vec3 delta = end - start;
            rail.length = glm::length(delta);
            rail.dir = (rail.length > 0.0f) ? delta / rail.length : glm::vec3(0.0f);
            return &rail;
        };

        auto addPair = [&](const glm::vec3 &a, const glm::vec3 &b)
        {
            makeRail(a, b);
            makeRail(b, a);
        };

        glm::vec3 i1(-20.0f, 0.0f, -20.0f);
        glm::vec3 i2(-20.0f, 0.0f, 0.0f);
        glm::vec3 i3(-20.0f, 0.0f, 20.0f);
        glm::vec3 i4(2.5f, 0.0f, -20.0f);
        glm::vec3 i5(2.5f, 0.0f, 0.0f);
        glm::vec3 i6(2.5f, 0.0f, 20.0f);
        glm::vec3 i7(38.0f, 0.0f, -20.0f);
        glm::vec3 i8(38.0f, 0.0f, 0.0f);
        glm::vec3 i9(38.0f, 0.0f, 20.0f);

        addPair(i1, i2);
        addPair(i2, i3);
        addPair(i4, i5);
        addPair(i5, i6);
        addPair(i7, i8);
        addPair(i8, i9);
        addPair(i1, i4);
        addPair(i4, i7);
        addPair(i2, i5);
        addPair(i5, i8);
        addPair(i3, i6);
        addPair(i6, i9);

        // Link rails
        for (auto &r : rails_local)
        {
            r.nextRails.clear();
            for (auto &candidate : rails_local)
            {
                if (&r == &candidate)
                    continue;
                if (closeEnough(r.endPosition, candidate.startPosition))
                {
                    r.nextRails.push_back(&candidate);
                }
            }
        }

        // Init Train
        Rail *startRail = nullptr;
        for (auto &r : rails_local)
        {
            if (closeEnough(r.startPosition, i2) && closeEnough(r.endPosition, i5))
            {
                startRail = &r;
                break;
            }
        }
        train.rail = startRail ? startRail : (rails_local.empty() ? nullptr : &rails_local.front());
        train.progress = 0.0f;
        train.speed = 4.0f;
        train.position = train.rail ? train.rail->startPosition : glm::vec3(0.0f);
        train.direction = train.rail ? train.rail->dir : glm::vec3(1.0f, 0.0f, 0.0f);
        train.yaw = DirToYaw(train.direction);
        train.numWagons = 2;
        train.waitingAtIntersection = false;

        // --- INIT SYMBOLS ---
        srand(static_cast<unsigned int>(time(nullptr)));
        glm::vec3 baseColors[3] = {
            glm::vec3(0.0f, 0.0f, 1.0f), // blue
            glm::vec3(0.0f, 1.0f, 0.0f), // green
            glm::vec3(0.0f, 1.0f, 1.0f)  // cyan
        };

        // Inițializare variabile logica de joc
        currentSymbolCount = 5;
        symbolsCollected = 0;
        collectingSymbol = false;
        collectTimer = 0.0f;
        symbolsToCollect = 0;
        isDelivering = false; // Folosim asta ca flag pentru faza de întoarcere

        symbolTypes.resize(currentSymbolCount);
        symbolOriginalColors.resize(currentSymbolCount);

        for (int i = 0; i < currentSymbolCount; i++)
        {
            symbolTypes[i] = rand() % 3;
            symbolOriginalColors[i] = baseColors[symbolTypes[i]];
            std::string meshName = "symbol" + std::to_string(i);
            float symbolSize = 1.0f;
            glm::vec3 center(0.0f);
            switch (symbolTypes[i])
            {
            case 0:
                AddMeshToList(object3D::CreateCube(meshName, center, symbolSize, symbolOriginalColors[i], true));
                break;
            case 1:
                AddMeshToList(object3D::CreatePyramidMesh(meshName, center - glm::vec3(0.0f, symbolSize / 2.0f, 0.0f), symbolSize, symbolSize, symbolOriginalColors[i], true));
                break;
            case 2:
                AddMeshToList(object3D::CreateSphere(meshName, center, symbolSize * 0.5f, symbolOriginalColors[i], true));
                break;
            }
        }

        symbolsInitialized = true;
    }

    void Tema2::FrameStart()
    {
        // Clears the color buffer (using the previously set color) and depth buffer
        glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::ivec2 resolution = window->GetResolution();
        // Sets the screen area where to draw
        glViewport(0, 0, resolution.x, resolution.y);
    }

    void Tema2::RenderMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix)
    {
        if (!mesh || !shader || !shader->program)
            return;

        shader->Use();
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        mesh->Render();
    }

    void Tema2::RenderRailAt(const glm::vec3 &pos, bool rotate90)
    {
        // Check if this position is an intersection point
        // Intersections are at: x = {-20, 2.5, 38}, z = {-20, 0, 20}
        auto isIntersection = [](float x, float z) -> bool
        {
            const float eps = 0.1f;
            bool xMatch = (std::abs(x - (-20.0f)) < eps) || (std::abs(x - 2.5f) < eps) || (std::abs(x - 38.0f) < eps);
            bool zMatch = (std::abs(z - (-20.0f)) < eps) || (std::abs(z - 0.0f) < eps) || (std::abs(z - 20.0f) < eps);
            return xMatch && zMatch;
        };

        // Get intersection mesh name based on position and type
        auto getIntersectionMeshName = [&](float x, float z) -> std::string
        {
            const float eps = 0.1f;
            std::string railType = getRailType(pos);

            // Determine which intersection
            if (std::abs(x - (-20.0f)) < eps)
            {
                if (std::abs(z - (-20.0f)) < eps)
                    return "intNormal_-20_-20";
                if (std::abs(z - 0.0f) < eps)
                    return "intTunnel_-20_0";
                if (std::abs(z - 20.0f) < eps)
                    return "intNormal_-20_20";
            }
            if (std::abs(x - 2.5f) < eps)
            {
                if (std::abs(z - (-20.0f)) < eps)
                    return "intNormal_2.5_-20";
                if (std::abs(z - 0.0f) < eps)
                    return "intNormal_2.5_0";
                if (std::abs(z - 20.0f) < eps)
                    return "intNormal_2.5_20";
            }
            if (std::abs(x - 38.0f) < eps)
            {
                if (std::abs(z - (-20.0f)) < eps)
                    return "intBridge_38_-20";
                if (std::abs(z - 0.0f) < eps)
                    return "intBridge_38_0";
                if (std::abs(z - 20.0f) < eps)
                    return "intBridge_38_20";
            }
            return "";
        };

        std::string meshName;
        bool useIntersectionMesh = false;

        if (isIntersection(pos.x, pos.z))
        {
            meshName = getIntersectionMeshName(pos.x, pos.z);
            useIntersectionMesh = !meshName.empty() && meshes.count(meshName);
        }

        if (!useIntersectionMesh)
        {
            meshName = getRailType(pos);
        }

        if (meshes.count(meshName))
        {
            glm::mat4 model = glm::mat4(1);
            model = glm::translate(model, pos);
            // Intersection meshes don't need rotation - they handle all directions
            if (rotate90 && !useIntersectionMesh)
            {
                model = model * transform3D::RotateOY(RADIANS(90.0f));
            }
            RenderMesh(meshes[meshName], shaders["VertexColor"], model);
        }
    }

    void Tema2::Update(float deltaTimeSeconds)
    {
        // Ending screen overlay
        if (endScreenActive)
        {
            timer += deltaTimeSeconds;
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (textRenderer)
            {
                glm::ivec2 res = window ? window->GetResolution() : glm::ivec2(1280, 720);
                const std::string msg = "GAME OVER";
                float scale = glm::clamp(std::min(res.x, res.y) / 720.0f, 0.5f, 3.0f);

                float textWidth = 0.0f;
                float maxSizeY = 0.0f;
                for (char c : msg)
                {
                    auto it = textRenderer->Characters.find(c);
                    if (it == textRenderer->Characters.end())
                        continue;
                    const auto &ch = it->second;
                    textWidth += (ch.Advance >> 6) * scale;
                    maxSizeY = std::max(maxSizeY, static_cast<float>(ch.Size.y));
                }
                float textHeight = maxSizeY * scale;
                float x = res.x * 0.5f - textWidth * 0.5f;
                float y = res.y * 0.5f - textHeight * 0.5f;
                textRenderer->RenderText(msg, x, y, scale, glm::vec3(1.0f, 0.0f, 0.0f));
            }

            if (timer >= endScreenTime)
            {
                window->Close();
            }
            return;
        }

        // Camera logic
        if (cameraCentered && camera)
        {
            glm::vec3 camPos = train.position + centeredOffset;
            camera->Set(camPos, train.position, glm::vec3(0.0f, 1.0f, 0.0f));
            camera->distanceToTarget = glm::length(centeredOffset);
        }

        elapsedTime += deltaTimeSeconds;

        // Station positions
        glm::vec3 stationPositions[3] = {
            glm::vec3(41.0f, 0.0f, -13.0f), // cubeStation
            glm::vec3(7.5f, 0.0f, 17.5f),   // pyramidStation
            glm::vec3(-23.0f, 0.0f, -13.0f) // sphereStation
        };

        // =========================================================
        // === LOGICA DE JOC ===
        // =========================================================

        // 1. Cronometru Global
        if (symbolsInitialized)
        {
            deliveryTimer += deltaTimeSeconds;
            if (deliveryTimer >= maxDeliveryTime)
            {
                std::cout << ">>> TIMP EXPIRAT! GAME OVER." << std::endl;
                endScreenActive = true;
                timer = 0.0f;
            }
        }

        // 2. Colectare Simboluri
        if (collectingSymbol)
        {
            collectTimer += deltaTimeSeconds;
            if (collectTimer >= kSymbolWaitTime)
            {
                symbolsCollected++;
                symbolsToCollect--;
                collectTimer = 0.0f;

                if (symbolsToCollect <= 0)
                {
                    collectingSymbol = false;
                }
            }
        }
        else if (symbolsInitialized && symbolsCollected < currentSymbolCount)
        {
            int nextSymbolType = symbolTypes[symbolsCollected];
            float dist = glm::length(glm::vec2(train.position.x, train.position.z) -
                                     glm::vec2(stationPositions[nextSymbolType].x, stationPositions[nextSymbolType].z));

            if (dist < kStationRadius)
            {
                int consecutiveCount = 0;
                for (int j = symbolsCollected; j < currentSymbolCount; j++)
                {
                    if (symbolTypes[j] == nextSymbolType)
                        consecutiveCount++;
                    else
                        break;
                }
                collectingSymbol = true;
                collectTimer = 0.0f;
                symbolsToCollect = consecutiveCount;
            }
        }

        // 3. Logica de Retur la Bază
        bool allCollected = (symbolsCollected >= currentSymbolCount);
        if (allCollected)
        {
            float distToMain = glm::length(glm::vec2(train.position.x, train.position.z) - glm::vec2(-2.5f, 0.0f));
            if (distToMain < 2.0f)
            {
                std::cout << ">>> COMANDA FINALIZATA! Generam nivelul urmator." << std::endl;
                currentSymbolCount++;
                deliveryTimer = 0.0f;

                glm::vec3 baseColors[3] = {glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), glm::vec3(0, 1, 1)};
                symbolTypes.resize(currentSymbolCount);
                symbolOriginalColors.resize(currentSymbolCount);

                for (int i = 0; i < currentSymbolCount; i++)
                {
                    symbolTypes[i] = rand() % 3;
                    symbolOriginalColors[i] = baseColors[symbolTypes[i]];
                    std::string meshName = "symbol" + std::to_string(i);
                    float symbolSize = 1.0f;
                    glm::vec3 center(0.0f);
                    switch (symbolTypes[i])
                    {
                    case 0:
                        AddMeshToList(object3D::CreateCube(meshName, center, symbolSize, symbolOriginalColors[i], true));
                        break;
                    case 1:
                        AddMeshToList(object3D::CreatePyramidMesh(meshName, center - glm::vec3(0.0f, symbolSize / 2.0f, 0.0f), symbolSize, symbolSize, symbolOriginalColors[i], true));
                        break;
                    case 2:
                        AddMeshToList(object3D::CreateSphere(meshName, center, symbolSize * 0.5f, symbolOriginalColors[i], true));
                        break;
                    }
                }
                symbolsCollected = 0;
                symbolsToCollect = 0;
                collectingSymbol = false;
            }
        }

        // =========================================================
        // === RANDARE ===
        // =========================================================

        glm::mat4 modelMatrix = glm::mat4(1);

        // Copaci
        for (float z = -50.0f; z <= 50.0f; z += 5.0f)
        {
            for (float x = -50.0f; x <= 50.0f; x += 5.0f)
            {
                if (((z == 0.0f || z == 20.0f || z == -20.0f) && x >= -20.0f && x <= 38.0f) || ((x == -20.0f || x == 38.0f) && z >= -20.0f && z <= 20.0f))
                    continue;
                modelMatrix = glm::mat4(1);
                modelMatrix = transform3D::Translate(x, 0.0f, z);
                RenderMesh(meshes["tree"], shaders["VertexColor"], modelMatrix);
            }
        }

        RenderMesh(meshes["ground"], shaders["VertexColor"], glm::mat4(1));
        modelMatrix = transform3D::Translate(30.0f, 0.0f, 0.0f);
        RenderMesh(meshes["water"], shaders["VertexColor"], modelMatrix);
        modelMatrix = transform3D::Translate(-15.0f, 0.0f, 0.0f);
        RenderMesh(meshes["mountain"], shaders["VertexColor"], modelMatrix);

        // --- TRAIN MOVEMENT LOGIC (FIXED) ---
        if (train.rail && !train.waitingAtIntersection && !collectingSymbol)
        {
            float safeLength = std::max(train.rail->length - 2.0f * kStopOffset, 0.0001f);
            glm::vec3 startMove = train.rail->startPosition + train.rail->dir * kStopOffset;
            glm::vec3 endStop = train.rail->endPosition - train.rail->dir * kStopOffset;

            train.progress += (train.speed * deltaTimeSeconds) / safeLength;

            if (train.progress >= 1.0f)
            {
                // Am ajuns la capătul șinei curente
                train.progress = 1.0f;
                train.position = endStop;
                train.direction = train.rail->dir;
                train.yaw = DirToYaw(train.direction);

                // Filtrăm opțiunile valide (excludem mersul direct înapoi)
                std::vector<Rail *> validOptions;
                for (Rail *nextR : train.rail->nextRails)
                {
                    // Calculăm dot product între direcția curentă și noua direcție
                    float dot = glm::dot(train.rail->dir, nextR->dir);
                    // Dacă dot > -0.75, înseamnă că nu e o întoarcere la 180 grade (care ar avea dot -1)
                    if (dot > -0.75f)
                    {
                        validOptions.push_back(nextR);
                    }
                }

                if (validOptions.empty())
                {
                    // Dead end
                    train.waitingAtIntersection = true;
                }
                else if (validOptions.size() == 1)
                {
                    // O SINGURĂ opțiune validă (colț sau linie dreaptă) -> Continuăm automat
                    train.rail = validOptions[0];

                    // Recalculăm poziția pentru noua șină
                    float nextSafeLength = std::max(train.rail->length - 2.0f * kStopOffset, 0.0001f);
                    glm::vec3 nextStartMove = train.rail->startPosition + train.rail->dir * kStopOffset;

                    // Proiectăm poziția curentă pe noua direcție pentru continuitate
                    float proj = glm::dot(train.position - nextStartMove, train.rail->dir);
                    train.progress = proj / nextSafeLength;

                    // Actualizăm direcția și rotația imediat
                    train.direction = train.rail->dir;
                    train.yaw = DirToYaw(train.direction);

                    // NU oprim trenul (waitingAtIntersection ramane false)
                }
                else
                {
                    // Intersecție reală (2 sau mai multe direcții posibile) -> STOP
                    train.waitingAtIntersection = true;
                }
            }
            else
            {
                train.position = startMove + train.rail->dir * (safeLength * train.progress);
                train.direction = train.rail->dir;
                train.yaw = DirToYaw(train.direction);
            }
        }

        // Render Train
        modelMatrix = glm::mat4(1);
        modelMatrix = transform3D::Translate(train.position.x, train.position.y, train.position.z) * transform3D::RotateOY(train.yaw);
        RenderMesh(meshes["locomotive"], shaders["VertexColor"], modelMatrix);

        for (int i = 0; i < train.numWagons; i++)
        {
            glm::vec3 wagonPos = train.position - train.direction * wagonLength * static_cast<float>(i + 1);
            modelMatrix = transform3D::Translate(wagonPos.x, wagonPos.y, wagonPos.z) * transform3D::RotateOY(train.yaw);
            RenderMesh(meshes["wagon"], shaders["VertexColor"], modelMatrix);
            modelMatrix = glm::mat4(1);
        }

        // Render Stations
        modelMatrix = transform3D::Translate(41.0f, 0.0f, -13.0f) * transform3D::RotateOY(RADIANS(-90.0f));
        RenderMesh(meshes["cubeStation"], shaders["VertexColor"], modelMatrix);
        modelMatrix = transform3D::Translate(7.5f, 0.0f, 17.5f);
        RenderMesh(meshes["pyramidStation"], shaders["VertexColor"], modelMatrix);
        modelMatrix = transform3D::Translate(-23.0f, 0.0f, -13.0f) * transform3D::RotateOY(RADIANS(90.0f));
        RenderMesh(meshes["sphereStation"], shaders["VertexColor"], modelMatrix);
        modelMatrix = transform3D::Translate(0.0f, 0.0f, 2.5f) * transform3D::RotateOY(RADIANS(180.0f));
        RenderMesh(meshes["mainStation"], shaders["VertexColor"], modelMatrix);

        // Render Symbols
        if (symbolsInitialized)
        {
            float t = glm::clamp(deliveryTimer / maxDeliveryTime, 0.0f, 1.0f);
            glm::vec3 redColor(1.0f, 0.0f, 0.0f);
            glm::vec3 groupCenter(0.0f, 4.0f, 2.5f);
            glm::vec3 toCamera = glm::normalize(glm::vec3(camera->position.x, 0.0f, camera->position.z) - glm::vec3(0.0f, 0.0f, 2.5f));
            float yaw = std::atan2(toCamera.x, toCamera.z);
            glm::vec3 rightDir = glm::vec3(-toCamera.z, 0.0f, toCamera.x);

            int remainingSymbols = currentSymbolCount - symbolsCollected;
            for (int i = symbolsCollected; i < currentSymbolCount; i++)
            {
                std::string meshName = "symbol" + std::to_string(i);
                if (meshes.count(meshName) && meshes[meshName])
                {
                    Mesh *symbolMesh = meshes[meshName];
                    glm::vec3 currentColor = glm::mix(symbolOriginalColors[i], redColor, t);
                    for (auto &v : symbolMesh->vertices)
                        if (glm::length(v.color) > 0.01f)
                            v.color = currentColor;
                    symbolMesh->InitFromData(symbolMesh->vertices, symbolMesh->indices);

                    int displayIndex = i - symbolsCollected;
                    float offset = -((displayIndex - (remainingSymbols - 1) / 2.0f) * 1.5f);
                    glm::vec3 symbolPos = groupCenter + rightDir * offset;
                    modelMatrix = transform3D::Translate(symbolPos.x, symbolPos.y, symbolPos.z) * transform3D::RotateOY(yaw);
                    RenderMesh(symbolMesh, shaders["VertexColor"], modelMatrix);
                }
            }
        }

        // Render Rails
        auto isIntersectionPos = [](float x, float z) -> bool
        {
            const float eps = 0.1f;
            return ((std::abs(x - (-20.0f)) < eps) || (std::abs(x - 2.5f) < eps) || (std::abs(x - 38.0f) < eps)) &&
                   ((std::abs(z - (-20.0f)) < eps) || (std::abs(z - 0.0f) < eps) || (std::abs(z - 20.0f) < eps));
        };

        float xLines[3] = {-20.0f, 2.5f, 38.0f};
        for (float xVal : xLines)
        {
            for (float z = -20.0f; z <= 20.0f; z += 2.0f)
            {
                if (isIntersectionPos(xVal, z) && std::abs(xVal - 2.5f) > 0.01f)
                    continue;
                RenderRailAt(glm::vec3(xVal, 0.0f, z), true);
            }
        }
        float zLines[3] = {-20.0f, 0.0f, 20.0f};
        for (float zVal : zLines)
        {
            for (float x = -20.0f; x <= 38.0f; x += 2.0f)
            {
                RenderRailAt(glm::vec3(x, 0.0f, zVal), false);
            }
        }

        // Text Return
        if (allCollected && textRenderer && !endScreenActive)
        {
            float timeLeft = std::max(0.0f, 1 + maxDeliveryTime - deliveryTimer);
            std::stringstream ss;
            ss << "RETURN: " << (int)timeLeft << "s";
            glm::vec3 pos(0.0f, 4.0f, 2.5f);
            glm::vec4 clipSpace = projectionMatrix * camera->GetViewMatrix() * glm::vec4(pos, 1.0f);
            if (clipSpace.w != 0.0f)
                clipSpace /= clipSpace.w;
            glm::ivec2 res = window->GetResolution();
            float x = (clipSpace.x * 0.5f + 0.5f) * res.x;
            float y = (1.0f - (clipSpace.y * 0.5f + 0.5f)) * res.y;
            textRenderer->RenderText(ss.str(), x - 100, y - 30, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        }
    }

    void Tema2::FrameEnd()
    {
        // Draw main scene coordinate system if needed
        // DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);

        // --- MINIMAP (bottom-right) ---
        if (window && !endScreenActive)
        {
            glm::ivec2 res = window->GetResolution();
            int size = std::min(res.x, res.y) / 2;

            if (size > 8)
            {
                int margin = minimapMargin;
                int vpX = res.x - margin - size;
                int vpY = margin;

                GLint prevViewport[4];
                glGetIntegerv(GL_VIEWPORT, prevViewport);
                glViewport(vpX, vpY, size, size);

                // Mărim zona vizibilă a hărții la 75 unități
                float mapSize = 75.0f;
                float halfSize = mapSize * 0.5f;

                // Centrăm proiecția mai bine
                glm::mat4 projMinimap = glm::ortho(-halfSize, halfSize, -halfSize, halfSize, -100.0f, 100.0f);

                glm::vec3 center(10.0f, 0.0f, 0.0f);
                glm::vec3 eye = center + glm::vec3(0.0f, 50.0f, 0.0f);
                glm::vec3 up = glm::vec3(0.0f, 0.0f, -1.0f);
                glm::mat4 viewMinimap = glm::lookAt(eye, center, up);

                Shader *shader = shaders["VertexColor"];
                if (shader && shader->program)
                {
                    GLboolean depthEnabled;
                    glGetBooleanv(GL_DEPTH_TEST, &depthEnabled);
                    glEnable(GL_DEPTH_TEST);
                    glClear(GL_DEPTH_BUFFER_BIT); // Important: curățăm depth buffer-ul local

                    shader->Use();

                    auto setUniforms = [&](const glm::mat4 &model)
                    {
                        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMinimap));
                        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projMinimap));
                        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(model));
                    };

                    // --- 1. ELEMENTE DE MEDIU (Decor) ---
                    if (meshes["ground"])
                    {
                        setUniforms(glm::mat4(1));
                        meshes["ground"]->Render();
                    }
                    if (meshes["water"])
                    {
                        setUniforms(transform3D::Translate(30.0f, 0.0f, 0.0f));
                        meshes["water"]->Render();
                    }
                    if (meshes["mountain"])
                    {
                        setUniforms(transform3D::Translate(-15.0f, 0.0f, 0.0f));
                        meshes["mountain"]->Render();
                    }

                    // Copaci (doar pe margini pentru a nu aglomera vizual)
                    for (float z = -50.0f; z <= 50.0f; z += 15.0f)
                    {
                        for (float x = -50.0f; x <= 50.0f; x += 15.0f)
                        {
                            if (abs(x) < 45 && abs(z) < 25)
                                continue; // Zona centrală liberă
                            setUniforms(transform3D::Translate(x, 0.0f, z));
                            meshes["tree"]->Render();
                        }
                    }

                    // --- MODIFICARE: Randare Șine (Cu linii perpendiculare pentru pod) ---
                    auto renderRailOnMinimap = [&](glm::vec3 pos, bool rotate90)
                    {
                        std::string name = getRailType(pos);

                        if (name == "bridgeRail")
                        {
                            // Randăm linii albe perpendiculare (traverse dese)
                            // Segmentul are lungime aprox 2.0. Desenăm 8 linii.
                            int numStripes = 8;
                            float segmentLength = 2.0f;
                            float step = segmentLength / (float)numStripes;
                            float startOffset = -segmentLength / 2.0f + step / 2.0f;

                            if (meshes["map_stripe"])
                            {
                                for (int k = 0; k < numStripes; k++)
                                {
                                    float offset = startOffset + k * step;
                                    glm::vec3 stripePos;
                                    glm::vec3 scale;

                                    if (rotate90)
                                    {
                                        // Șina e pe axa X. Liniile trebuie să fie perpendiculare (pe Z).
                                        // offset-ul e pe X.
                                        stripePos = pos + glm::vec3(offset, 0.5f, 0.0f);
                                        // Scalăm pătratul: Subțire pe X (0.1), Lat pe Z (1.0)
                                        scale = glm::vec3(0.05f, 0.5f, 0.5f); 
                                    }
                                    else
                                    {
                                        // Șina e pe axa Z. Liniile trebuie să fie perpendiculare (pe X).
                                        // offset-ul e pe Z.
                                        stripePos = pos + glm::vec3(0.0f, 0.5f, offset);
                                        // Scalăm pătratul: Lat pe X (1.0), Subțire pe Z (0.1)
                                        scale = glm::vec3(0.5f, 0.5f, 0.05f);
                                    }

                                    glm::mat4 m = glm::translate(glm::mat4(1), stripePos);
                                    // Pătratul e în XY, îl culcăm pe XZ (rotate 90 OX)
                                    m = m * transform3D::RotateOX(RADIANS(90.0f));
                                    m = glm::scale(m, scale);
                                    
                                    setUniforms(m);
                                    meshes["map_stripe"]->Render();
                                }
                            }
                        }
                        else
                        {
                            // Șină normală sau tunel - comportament standard
                            if (meshes[name])
                            {
                                glm::mat4 m = glm::translate(glm::mat4(1), pos);
                                if (rotate90) m = m * transform3D::RotateOY(RADIANS(90.0f));
                                setUniforms(m);
                                meshes[name]->Render();
                            }
                        }
                    };

                    float xLines[3] = {-20.0f, 2.5f, 38.0f};
                    for (float xVal : xLines)
                    {
                        for (float z = -20.0f; z <= 20.0f; z += 2.0f)
                        {
                            if (std::abs(xVal - 2.5f) < 0.1f && (z < -20 || z > 20))
                                continue;
                            renderRailOnMinimap(glm::vec3(xVal, 0, z), true);
                        }
                    }
                    float zLines[3] = {-20.0f, 0.0f, 20.0f};
                    for (float zVal : zLines)
                    {
                        for (float x = -20.0f; x <= 38.0f; x += 2.0f)
                        {
                            renderRailOnMinimap(glm::vec3(x, 0, zVal), false);
                        }
                    }

                    // --- 2. SIMBOLURI STATICE ---
                    
                    // A. Gara Centrală -> STEA MOV
                    {
                        float ratio = 0.0f;
                        if (maxDeliveryTime > 0.0f)
                            ratio = glm::clamp(deliveryTimer / maxDeliveryTime, 0.0f, 1.0f);

                        Mesh *starMesh = meshes["map_star"];
                        if (starMesh)
                        {
                            for (auto &v : starMesh->vertices)
                                v.color = glm::vec3(0.6f, 0.1f, 0.6f);
                            starMesh->InitFromData(starMesh->vertices, starMesh->indices);

                            glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(0.0f, 10.0f, 2.5f));
                            model = glm::scale(model, glm::vec3(3.5f));
                            model = model * transform3D::RotateOX(RADIANS(90.0f));
                            setUniforms(model);
                            starMesh->Render();
                        }
                    }

                    // B. Stația Cub (X=41) -> PĂTRAT ALBASTRU
                    {
                        glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(41.0f, 10.0f, -13.0f));
                        model = glm::scale(model, glm::vec3(4.0f));
                        model = model * transform3D::RotateOX(RADIANS(90.0f));
                        setUniforms(model);
                        if (meshes["map_square"])
                            meshes["map_square"]->Render();
                    }

                    // C. Stația Piramidă (X=7.5) -> TRIUNGHI VERDE
                    {
                        glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(7.5f, 10.0f, 17.5f));
                        model = glm::scale(model, glm::vec3(4.0f));
                        model = model * transform3D::RotateOX(RADIANS(90.0f));
                        setUniforms(model);
                        if (meshes["map_triangle"])
                            meshes["map_triangle"]->Render();
                    }

                    // D. Stația Sferă (X=-23) -> CERC TURCUAZ
                    {
                        glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(-23.0f, 10.0f, -13.0f));
                        model = glm::scale(model, glm::vec3(3.0f));
                        model = model * transform3D::RotateOX(RADIANS(90.0f));
                        setUniforms(model);
                        if (meshes["map_circle"])
                            meshes["map_circle"]->Render();
                    }

                    // --- 3. ELEMENTE DINAMICE (Tren, Săgeată) ---

                    // E. Trenul
                    if (meshes["map_train"])
                    {
                        glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(train.position.x, 11.0f, train.position.z));
                        model = glm::rotate(model, train.yaw + RADIANS(90.0f), glm::vec3(0, 1, 0));
                        model = glm::scale(model, glm::vec3(2.0f, 1.0f, 5.0f));
                        model = model * transform3D::RotateOX(RADIANS(90.0f));
                        setUniforms(model);
                        meshes["map_train"]->Render();
                    }

                    // F. Săgeată Cameră - !!! MODIFICARE: POZITIE CAMERA, NU TREN
                    if (meshes["map_arrow"])
                    {
                        // Folosim poziția camerei (X, Z) și înălțimea fixă (12.0f)
                        glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(camera->position.x, 12.0f, camera->position.z));

                        // Orientarea săgeții trebuie să fie în funcție de direcția camerei
                        glm::vec3 camDir = glm::normalize(glm::vec3(camera->forward.x, 0.0f, camera->forward.z));
                        float arrowYaw = std::atan2(camDir.x, camDir.z) + RADIANS(180.0f);

                        model = glm::rotate(model, arrowYaw, glm::vec3(0, 1, 0));
                        model = model * transform3D::RotateOX(RADIANS(90.0f));
                        model = model * transform3D::RotateOZ(RADIANS(180.0f));

                        model = glm::scale(model, glm::vec3(4.0f));
                        setUniforms(model);
                        meshes["map_arrow"]->Render();
                    }

                    if (!depthEnabled)
                        glDisable(GL_DEPTH_TEST);
                }
                glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
            }

            // HUD timer (top-right, offset by minimap margin)
            if (textRenderer)
            {
                // Scale with the smallest screen dimension for variation similar to minimap sizing
                float scale = glm::clamp((std::min(res.x, res.y) / 720.0f) * 0.55f, 0.25f, 2.4f);
                int totalSeconds = static_cast<int>(elapsedTime);
                int hours = totalSeconds / 3600;
                int minutes = (totalSeconds / 60) % 60;
                int seconds = totalSeconds % 60;

                std::ostringstream oss;
                oss << std::setfill('0') << std::setw(2) << hours << "h:";
                oss << std::setfill('0') << std::setw(2) << minutes << "m:";
                oss << std::setfill('0') << std::setw(2) << seconds << "s";
                std::string text = oss.str();

                // measure text width/height
                float textWidth = 0.0f;
                float maxSizeY = 0.0f;
                for (char c : text)
                {
                    auto it = textRenderer->Characters.find(c);
                    if (it == textRenderer->Characters.end())
                        continue;
                    const auto &ch = it->second;
                    textWidth += (ch.Advance >> 6) * scale;
                    maxSizeY = std::max(maxSizeY, static_cast<float>(ch.Size.y));
                }

                float x = res.x - minimapMargin - textWidth;
                // Place at top-right with same margin as minimap (TextRenderer origin is top-left style)
                float y = minimapMargin;
                textRenderer->RenderText(text, x, y, scale, hudTimerColor);
            }
        }
    }

    /*
     *  These are callback functions. To find more about callbacks and
     *  how they behave, see `input_controller.h`.
     */

    void Tema2::OnInputUpdate(float deltaTime, int mods)
    {
        // move the camera only if MOUSE_RIGHT button is pressed
        if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
        {
            float cameraSpeed = 2.5f;

            // Sprint cu Shift stanga
            if (window->KeyHold(GLFW_KEY_LEFT_SHIFT))
            {
                cameraSpeed *= 4.0f;
            }

            if (window->KeyHold(GLFW_KEY_W))
            {
                // TranslateForward moves along the camera's forward vector (respects pitch)
                camera->TranslateForward(cameraSpeed * deltaTime);
            }

            if (window->KeyHold(GLFW_KEY_A))
            {
                camera->TranslateRight(-cameraSpeed * deltaTime);
            }

            if (window->KeyHold(GLFW_KEY_S))
            {
                // Move backward along camera forward (respects pitch)
                camera->TranslateForward(-cameraSpeed * deltaTime);
            }

            if (window->KeyHold(GLFW_KEY_D))
            {
                camera->TranslateRight(cameraSpeed * deltaTime);
            }

            if (window->KeyHold(GLFW_KEY_Q))
            {
                camera->TranslateUpward(-cameraSpeed * deltaTime);
            }

            if (window->KeyHold(GLFW_KEY_E))
            {
                camera->TranslateUpward(cameraSpeed * deltaTime);
            }
        }

        // Adjust train speed with arrow keys (held)
        const float accel = 4.0f;
        const float minSpeed = 0.1f;
        const float maxSpeed = 20.0f;
        if (window->KeyHold(GLFW_KEY_UP))
        {
            train.speed = glm::clamp(train.speed + accel * deltaTime, minSpeed, maxSpeed);
        }
        if (window->KeyHold(GLFW_KEY_DOWN))
        {
            train.speed = glm::clamp(train.speed - accel * deltaTime, minSpeed, maxSpeed);
        }
    }

    void Tema2::OnKeyPress(int key, int mods)
    {
        // Choose path at intersections only when camera follows the train
        if (train.waitingAtIntersection && cameraCentered && train.rail)
        {
            auto pickDirection = [&](const glm::vec3 &desired, bool rotate) -> bool
            {
                Rail *best = nullptr;
                float bestDot = 0.3f; // small threshold to avoid wrong turns
                glm::vec3 desiredDir = glm::normalize(desired);

                for (Rail *candidate : train.rail->nextRails)
                {
                    glm::vec3 candDir = glm::normalize(candidate->dir);
                    float d = glm::dot(desiredDir, candDir);
                    if (d > bestDot)
                    {
                        bestDot = d;
                        best = candidate;
                    }
                }

                if (best)
                {
                    // Preserve the current distance to the node so departure starts at the same offset
                    train.rail = best;
                    float nextSafeLength = std::max(train.rail->length - 2.0f * kStopOffset, 0.0001f);
                    glm::vec3 nextStartMove = train.rail->startPosition + train.rail->dir * kStopOffset;
                    float proj = glm::dot(train.position - nextStartMove, train.rail->dir);

                    // trebuie sa adaug o animatie pentru fiecare componenta aici
                    if (rotate)
                        train.progress = 0.25f / nextSafeLength;
                    else
                        train.progress = proj / nextSafeLength;

                    train.position = train.rail->startPosition - train.rail->dir * kStopOffset;
                    train.direction = train.rail->dir;
                    train.yaw = DirToYaw(train.direction);
                    train.waitingAtIntersection = false;
                    return true;
                }
                return false;
            };

            glm::vec3 forward = glm::normalize(train.rail->dir);
            if (glm::length(forward) < 0.0001f)
                forward = glm::vec3(1.0f, 0.0f, 0.0f);
            glm::vec3 left = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), forward));
            glm::vec3 right = -left;

            if (key == GLFW_KEY_W && pickDirection(forward, false))
                return;
            if (key == GLFW_KEY_A && pickDirection(left, true))
                return;
            if (key == GLFW_KEY_D && pickDirection(right, true))
                return;
        }

        // Toggle camera mode with 'C' key: free <-> centered on locomotive
        if (key == GLFW_KEY_C)
        {
            cameraCentered = !cameraCentered;
            if (cameraCentered)
            {
                // Fiecare intrare în modul follow folosește aceeași poziție relativă la locomotivă
                glm::vec3 locoPos = train.position + glm::vec3(0.0f, 0.6f, 0.0f);
                centeredOffset = glm::vec3(0.0f, 6.0f, 8.0f); // offset fix
                if (camera)
                {
                    glm::vec3 camPos = locoPos + centeredOffset;
                    camera->Set(camPos, locoPos, glm::vec3(0.0f, 1.0f, 0.0f));
                    camera->distanceToTarget = glm::length(centeredOffset);
                }
            }
            else
            {
                if (camera)
                {
                    // păstrează poziția curentă (ultima poziție de tren) dar decuplează targetul
                    glm::vec3 pos = camera->position;
                    glm::vec3 fwd = camera->forward;
                    glm::vec3 up = camera->up;
                    camera->Set(pos, pos + fwd, up);
                    camera->distanceToTarget = glm::length(fwd);
                }
            }
        }

        // Trigger ending screen with END key (avoid ESC which may close window immediately)
        if (key == GLFW_KEY_END)
        {
            endScreenActive = true;
            timer = 0.0f;
        }
    }

    void Tema2::OnKeyRelease(int key, int mods)
    {
        // Add key release event
    }

    void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
    {
        // Add mouse move event

        if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
        {
            float sensivityOX = 0.001f;
            float sensivityOY = 0.001f;

            if (cameraCentered)
            {
                // Orbit the camera around the locomotive by rotating the offset vector
                // Yaw (rotate around world up)
                float angleYaw = -sensivityOX * deltaX;
                centeredOffset = glm::vec3(glm::rotate(glm::mat4(1.0f), angleYaw, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(centeredOffset, 1.0f));

                // Pitch (rotate around right axis of the orbit)
                glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
                glm::vec3 dir = glm::normalize(centeredOffset);
                glm::vec3 rightAxis = glm::normalize(glm::cross(dir, worldUp));
                float anglePitch = -sensivityOY * deltaY;
                // Prevent degenerate axis
                if (glm::length(rightAxis) > 0.0001f)
                    centeredOffset = glm::vec3(glm::rotate(glm::mat4(1.0f), anglePitch, rightAxis) * glm::vec4(centeredOffset, 1.0f));
            }
            else
            {
                float sensOX = sensivityOX;
                float sensOY = sensivityOY;
                camera->RotateFirstPerson_OX(-sensOY * deltaY);
                camera->RotateFirstPerson_OY(-sensOX * deltaX);
            }
        }
    }

    void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
    {
        // Add mouse button press event
    }

    void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
    {
        // Add mouse button release event
    }

    void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
    {
        // Allow zoom only in follow mode (scroll changes distance to locomotive)
        if (cameraCentered)
        {
            float len = glm::length(centeredOffset);
            if (len > 0.0001f)
            {
                float zoomStep = 0.5f;
                float minDist = 1.0f;
                float maxDist = 40.0f;
                float newLen = glm::clamp(len - static_cast<float>(offsetY) * zoomStep, minDist, maxDist);

                glm::vec3 dir = centeredOffset / len;
                centeredOffset = dir * newLen;

                if (camera)
                {
                    glm::vec3 camPos = train.position + centeredOffset;
                    camera->Set(camPos, train.position, glm::vec3(0.0f, 1.0f, 0.0f));
                    camera->distanceToTarget = newLen;
                }
            }
        }
    }

    void Tema2::OnWindowResize(int width, int height)
    {
        // Update projection on resize to keep aspect consistent
        // Guard against height == 0 (can happen when window is minimized) and against tiny aspect values
        float aspect = 16.0f / 9.0f;
        if (height > 0)
        {
            aspect = static_cast<float>(width) / static_cast<float>(height);
        }
        else if (window && std::abs(window->props.aspectRatio) > std::numeric_limits<float>::epsilon())
        {
            aspect = window->props.aspectRatio;
        }
        if (std::abs(aspect) <= std::numeric_limits<float>::epsilon())
            aspect = 1.0f;

        projectionMatrix = glm::perspective(fovY, aspect, zNear, zFar);

        // Recreate text renderer to match new resolution
        glm::ivec2 res = window ? window->GetResolution() : glm::ivec2(width, height);
        delete textRenderer;
        textRenderer = new gfxc::TextRenderer(window->props.selfDir, res.x, res.y);
        textRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, fontPath), 64);
    }

} // namespace m1
