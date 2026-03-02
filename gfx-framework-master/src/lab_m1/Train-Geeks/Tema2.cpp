#include "lab_m1/Train-Geeks/Tema2.hpp"
#include "lab_m1/Train-Geeks/object3D.cpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;

namespace m1
{

    Tema2::Tema2() {}

    Tema2::~Tema2()
    {
        delete textRenderer;
        textRenderer = nullptr;
    }

    float Tema2::DirToYaw(const glm::vec3 &dir)
    {
        // Align yaw to mesh forward: +X -> 0 deg, adjust to avoid 180 flips on turns
        return std::atan2(dir.x, dir.z) - glm::half_pi<float>();
    }

    std::string Tema2::getRailType(glm::vec3 pos)
    {
        if (pos.x >= 20 && pos.x <= 40 && pos.z >= -20 && pos.z <= 20)
            return "bridgeRail"; // water area
        if (pos.x >= -22.5f && pos.x <= -7.5f && pos.z >= -15 && pos.z <= 15)
            return "tunnelRail"; // mountain area
        return "basicRail";      // normal rail
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

    void Tema2::Init()
    {
        // Text renderer for ending screen and HUD
        glm::ivec2 resolution = window ? window->GetResolution() : glm::ivec2(1280, 720);
        delete textRenderer;
        textRenderer = new gfxc::TextRenderer(window->props.selfDir, resolution.x, resolution.y);
        textRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, fontPath), 64);

        camera = new implemented::Camera();
        camera->Set(glm::vec3(0, 3, 7), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
        // Up = the axis that points above (Y axis); Forward = the direction the camera is looking;

        if (window)
        {
            const auto &res = window->props.resolution;
            if (res.y != 0)
                aspect = static_cast<float>(res.x) / static_cast<float>(res.y);
            else if (std::abs(window->props.aspectRatio) > std::numeric_limits<float>::epsilon())
                aspect = window->props.aspectRatio;
        }
        if (std::abs(aspect) <= std::numeric_limits<float>::epsilon())
            aspect = 1.0f;

        projectionMatrix = glm::perspective(fovY, aspect, zNear, zFar);

        cameraCentered = false;
        centeredOffset = camera->position - glm::vec3(0.0f, 0.6f, 0.0f);

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

        // Turquoise Circle (Sphere)
        Mesh *mapCircle = object3D::CreateCircle("map_circle", glm::vec3(0, 1, 1));
        AddMeshToList(mapCircle);

        // Green Triangle (Pyramid)
        Mesh *mapTriangle = object3D::CreateTriangle("map_triangle", glm::vec3(0, 1, 0));
        AddMeshToList(mapTriangle);

        // Blue Square (Cube)
        Mesh *mapSquare = object3D::CreateSquare("map_square", glm::vec3(0, 0, 1));
        AddMeshToList(mapSquare);

        // Purple Star for minimap
        Mesh *mapStar = object3D::CreateStar("map_star", glm::vec3(0.6f, 0.0f, 0.8f));
        AddMeshToList(mapStar);

        // Arrow and Train for minimap
        Mesh *mapArrow = object3D::CreateArrow("map_arrow", glm::vec3(1, 1, 1));
        AddMeshToList(mapArrow);

        Mesh *mapTrain = object3D::CreateSquare("map_train", glm::vec3(1.0f, 1.0f, 0.3f));
        AddMeshToList(mapTrain);

        // Stripe GREY for minimap
        Mesh *mapStripe = object3D::CreateSquare("map_stripe", glm::vec3(0.3, 0.3, 0.3));
        AddMeshToList(mapStripe);

        // INTERSECTIONS
        glm::vec3 railCol(0.3f, 0.3f, 0.3f);
        glm::vec3 tunnelCol(0.2f, 0.2f, 0.25f);
        glm::vec3 bridgeCol(0.5f, 0.4f, 0.3f);

        AddMeshToList(object3D::CreateNormalRailIntersection("intNormal_-20_-20", railCol, true, false, true, false));
        AddMeshToList(object3D::CreateNormalRailIntersection("intNormal_-20_20", railCol, false, true, true, false));
        AddMeshToList(object3D::CreateBridgeRailIntersection("intBridge_38_-20", railCol, bridgeCol, true, false, false, true));
        AddMeshToList(object3D::CreateBridgeRailIntersection("intBridge_38_20", railCol, bridgeCol, false, true, false, true));
        AddMeshToList(object3D::CreateTunnelRailIntersection("intTunnel_-20_0", railCol, tunnelCol, true, true, true, false));
        AddMeshToList(object3D::CreateBridgeRailIntersection("intBridge_38_0", railCol, bridgeCol, true, true, false, true));

        // Central intersections
        AddMeshToList(object3D::CreateNormalRailIntersection("intNormal_2.5_-20", railCol, true, false, true, true));
        AddMeshToList(object3D::CreateNormalRailIntersection("intNormal_2.5_20", railCol, false, true, true, true));
        AddMeshToList(object3D::CreateNormalRailIntersection("intNormal_2.5_0", railCol, true, true, true, true));

        // RAIL NETWORK BUILD
        rails.clear();
        rails.reserve(24);

        auto closeEnough = [](const glm::vec3 &a, const glm::vec3 &b, float eps = 0.01f)
        {
            return glm::length(a - b) <= eps;
        };

        auto makeRail = [&](const glm::vec3 &start, const glm::vec3 &end) -> Rail *
        {
            rails.push_back({});
            Rail &rail = rails.back();
            rail.startPosition = start;
            rail.endPosition = end;
            glm::vec3 delta = end - start;
            rail.length = glm::length(delta);
            rail.direction = (rail.length > 0.0f) ? delta / rail.length : glm::vec3(0.0f);
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
        for (auto &r : rails)
        {
            r.nextRails.clear();
            for (auto &candidate : rails)
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
        for (auto &r : rails)
        {
            if (closeEnough(r.startPosition, i2) && closeEnough(r.endPosition, i5))
            {
                startRail = &r;
                break;
            }
        }
        train.rail = startRail ? startRail : (rails.empty() ? nullptr : &rails.front());
        g_prevRail = nullptr; // no previous rail at start
        train.progress = 0.0f;
        train.speed = 4.0f;
        train.position = train.rail ? train.rail->startPosition : glm::vec3(0.0f);
        train.direction = train.rail ? train.rail->direction : glm::vec3(1.0f, 0.0f, 0.0f);
        train.yaw = DirToYaw(train.direction);
        train.numWagons = 2;
        train.waitingAtIntersection = false;

        // INIT SYMBOLS
        srand(static_cast<unsigned int>(time(nullptr)));
        glm::vec3 baseColors[3] = {
            glm::vec3(0.0f, 0.0f, 1.0f), // blue
            glm::vec3(0.0f, 1.0f, 0.0f), // green
            glm::vec3(0.0f, 1.0f, 1.0f)  // cyan
        };

        // Initialize game logic variables
        currentSymbolCount = 5;
        symbolsCollected = 0;
        collectingSymbol = false;
        collectTimer = 0.0f;
        symbolsToCollect = 0;
        isDelivering = false; // Use this as a flag for the return phase

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
                // Scale text based on resolution
                float scale = glm::clamp(std::min(res.x, res.y) / 720.0f, 0.5f, 3.0f);

                float textWidth = 0.0f;
                float maxSizeY = 0.0f;
                for (char c : msg)
                {
                    auto it = textRenderer->Characters.find(c);
                    if (it == textRenderer->Characters.end())
                        continue;
                    const auto &ch = it->second;
                    // width, character position (1/64 pixels)
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
        }

        elapsedTime += deltaTimeSeconds;

        // Global Timer
        if (symbolsInitialized)
        {
            deliveryTimer += deltaTimeSeconds;
            if (deliveryTimer >= maxDeliveryTime)
            {
                std::cout << ">>> TIME UP! GAME OVER." << std::endl;
                endScreenActive = true;
                timer = 0.0f;
            }
        }

        // Symbol Collection
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

        // Return to main station check
        bool allCollected = (symbolsCollected >= currentSymbolCount);
        if (allCollected)
        {
            float distToMain = glm::length(glm::vec2(train.position.x, train.position.z) - glm::vec2(-2.5f, 0.0f));
            if (distToMain < 2.0f)
            {
                std::cout << ">>> ORDER COMPLETED! Generating next level." << std::endl;
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

        // RENDER SCENE OBJECTS

        glm::mat4 modelMatrix = glm::mat4(1);

        // Trees
        for (float z = -50.0f; z <= 50.0f; z += 5.0f)
        {
            for (float x = -50.0f; x <= 50.0f; x += 5.0f)
            {
                if (((z == 0.0f || z == 20.0f || z == -20.0f) && x >= -20.0f && x <= 38.0f) || ((x == -20.0f || x == 38.0f) && z >= -20.0f && z <= 20.0f) || (x > 20.0f && x < 40.0f && z > -20.0f && z < 20.0f))
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

        // TRAIN MOVEMENT LOGIC
        if (train.rail && !train.waitingAtIntersection && !collectingSymbol)
        {
            // safeLength excludes stop offsets at both ends
            float safeLength = std::max(train.rail->length - 2.0f * kStopOffset, 0.0001f);
            glm::vec3 startMove = train.rail->startPosition + train.rail->direction * kStopOffset;
            glm::vec3 endStop = train.rail->endPosition - train.rail->direction * kStopOffset;

            train.progress += (train.speed * deltaTimeSeconds) / safeLength;

            if (train.progress >= 1.0f)
            {
                // Reached end of rail segment
                train.progress = 1.0f;
                train.position = endStop;
                train.direction = train.rail->direction;
                train.yaw = DirToYaw(train.direction);

                std::vector<Rail *> validOptions;
                for (Rail *nextR : train.rail->nextRails)
                {
                    float dot = glm::dot(train.rail->direction, nextR->direction);
                    // Avoid 180 degree turns
                    if (dot > -0.75f)
                    {
                        validOptions.push_back(nextR);
                    }
                }

                if (validOptions.empty())
                {
                    train.waitingAtIntersection = true;
                }
                else if (validOptions.size() == 1)
                {
                    // Select the only available option automatically
                    g_prevRail = train.rail;
                    train.rail = validOptions[0];

                    float nextSafeLength = std::max(train.rail->length - 2.0f * kStopOffset, 0.0001f);
                    glm::vec3 nextStartMove = train.rail->startPosition + train.rail->direction * kStopOffset;

                    float proj = glm::dot(train.position - nextStartMove, train.rail->direction);
                    // Resetăm progresul pentru noua șină
                    train.progress = proj / nextSafeLength;
                    if (train.progress < 0)
                        train.progress = 0;

                    train.direction = train.rail->direction;
                    train.yaw = DirToYaw(train.direction);
                }
                else
                {
                    train.waitingAtIntersection = true;
                }
            }
            else
            {
                train.position = startMove + train.rail->direction * (safeLength * train.progress);
                train.direction = train.rail->direction;
                train.yaw = DirToYaw(train.direction);
            }
        }

        // RENDER TRAIN

        // Locomotive (always at train.position)
        modelMatrix = glm::mat4(1);
        modelMatrix = transform3D::Translate(train.position.x, train.position.y, train.position.z) * transform3D::RotateOY(train.yaw);
        RenderMesh(meshes["locomotive"], shaders["VertexColor"], modelMatrix);

        // Wagons positioning
        auto calculateWagonState = [&](float distBehindHead, glm::vec3 &outPos, float &outYaw)
        {
            if (!train.rail)
                return;

            float currentRailDist = train.progress * (train.rail->length - 2.0f * kStopOffset) + kStopOffset;

            // logic for teleportation
            if (distBehindHead <= (currentRailDist - kStopOffset))
            {
                outPos = train.position - train.direction * distBehindHead;
                outYaw = train.yaw;
            }
            else
            {
                float remainingDist = distBehindHead - currentRailDist;

                if (g_prevRail)
                {
                    glm::vec3 junction = g_prevRail->endPosition;
                    // Check if remainingDist exceeds previous rail length
                    outPos = junction - g_prevRail->direction * (remainingDist + 2.0f * kStopOffset);
                    outYaw = DirToYaw(g_prevRail->direction);
                }
                else
                {
                    outPos = train.position - train.direction * distBehindHead;
                    outYaw = train.yaw;
                }
            }
        };

        for (int i = 0; i < train.numWagons; i++)
        {
            glm::vec3 wPos;
            float wYaw;
            float dist = wagonLength * (float)(i + 1);

            calculateWagonState(dist, wPos, wYaw);

            modelMatrix = glm::mat4(1);
            modelMatrix = transform3D::Translate(wPos.x, wPos.y, wPos.z) * transform3D::RotateOY(wYaw);
            RenderMesh(meshes["wagon"], shaders["VertexColor"], modelMatrix);
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

            // Calculate facing direction towards camera
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
                    
                    // Update vertex colors instead of using a new shader for simplicity
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

        // TEXT RENDERING - RETURN TIMER
        if (allCollected && textRenderer && !endScreenActive)
        {
            float timeLeft = std::max(0.0f, 1 + maxDeliveryTime - deliveryTimer);
            std::stringstream ss;
            ss << "RETURN: " << (int)timeLeft << "s";
            std::string text = ss.str();

            // 1. Calculate screen position
            glm::vec3 pos(0.0f, 4.0f, 2.5f);
            // position the text facing the camera
            glm::vec4 clipSpace = projectionMatrix * camera->GetViewMatrix() * glm::vec4(pos, 1.0f);
            if (clipSpace.w != 0.0f)
                clipSpace /= clipSpace.w; // normalize
            glm::ivec2 res = window->GetResolution();
            // Calculate text position in screen coordinates
            float x = (clipSpace.x * 0.5f + 0.5f) * res.x;
            float y = (1.0f - (clipSpace.y * 0.5f + 0.5f)) * res.y;

            // 2. Calculate scale (resizeable) based on resolution
            float scale = glm::clamp(std::min(res.x, res.y) / 720.0f, 0.5f, 3.0f);

            // 3. Calculate text width for centering
            float textWidth = 0.0f;
            for (char c : text)
            {
                auto it = textRenderer->Characters.find(c);
                if (it == textRenderer->Characters.end())
                    continue;
                textWidth += (it->second.Advance >> 6) * scale;
            }

            // position the text centered horizontally at the symbols location
            textRenderer->RenderText(text, x - textWidth / 2.0f, y - 20.0f * scale, scale, glm::vec3(1.0f, 0.0f, 0.0f));
        }
    }

    void Tema2::FrameEnd()
    {
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

                float mapSize = 75.0f;
                float halfSize = mapSize * 0.5f;

                // Center the projection better around the rail network
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
                    glClear(GL_DEPTH_BUFFER_BIT);

                    shader->Use();

                    auto setUniforms = [&](const glm::mat4 &model)
                    {
                        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMinimap));
                        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projMinimap));
                        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(model));
                    };

                    // use ->Render() and not RenderMesh because want it to be independent of the main camera and to allow 3D

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

                    auto renderRailOnMinimap = [&](glm::vec3 pos, bool rotate90)
                    {
                        Mesh *mesh = meshes["map_stripe"];
                        if (!mesh)
                            return;

                        float standardLen = 2.0f;
                        float thickness = 0.8f;
                        float edgeLen = 1.4f;
                        float overhang = 0.4f;
                        float edgeOffset = (edgeLen / 2.0f) - overhang;

                        float scaleLen = standardLen;
                        glm::vec3 drawPos = pos;
                        float eps = 0.1f;

                        if (rotate90)
                        {
                            if (std::abs(pos.z - (-20.0f)) < eps)
                            {
                                drawPos.z += edgeOffset;
                                scaleLen = edgeLen;
                            }
                            else if (std::abs(pos.z - 20.0f) < eps)
                            {
                                drawPos.z -= edgeOffset;
                                scaleLen = edgeLen;
                            }
                        }
                        else
                        {
                            if (std::abs(pos.x - (-20.0f)) < eps)
                            {
                                drawPos.x += edgeOffset;
                                scaleLen = edgeLen;
                            }
                            else if (std::abs(pos.x - 38.0f) < eps)
                            {
                                drawPos.x -= edgeOffset;
                                scaleLen = edgeLen;
                            }
                        }

                        glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(drawPos.x, 10.0f, drawPos.z));
                        if (rotate90)
                            model = model * transform3D::RotateOY(RADIANS(90.0f));
                        model = model * transform3D::RotateOX(RADIANS(90.0f));
                        model = glm::scale(model, glm::vec3(scaleLen, thickness, 1.0f));
                        setUniforms(model);
                        mesh->Render();
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

                    // Stations symbols
                    {
                        Mesh *starMesh = meshes["map_star"];
                        if (starMesh)
                        {
                            // Update vertex colors instead of using a new shader for simplicity
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
                    {
                        glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(41.0f, 10.0f, -13.0f));
                        model = glm::scale(model, glm::vec3(4.0f));
                        model = model * transform3D::RotateOX(RADIANS(90.0f));
                        setUniforms(model);
                        if (meshes["map_square"])
                            meshes["map_square"]->Render();
                    }
                    {
                        glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(7.5f, 10.0f, 17.5f));
                        model = glm::scale(model, glm::vec3(4.0f));
                        model = model * transform3D::RotateOX(RADIANS(90.0f));
                        setUniforms(model);
                        if (meshes["map_triangle"])
                            meshes["map_triangle"]->Render();
                    }
                    {
                        glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(-23.0f, 10.0f, -13.0f));
                        model = glm::scale(model, glm::vec3(3.0f));
                        model = model * transform3D::RotateOX(RADIANS(90.0f));
                        setUniforms(model);
                        if (meshes["map_circle"])
                            meshes["map_circle"]->Render();
                    }

                    // Dynamic elements: train and camera arrow
                    if (meshes["map_train"])
                    {
                        glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(train.position.x, 11.0f, train.position.z));
                        model = glm::rotate(model, train.yaw + RADIANS(90.0f), glm::vec3(0, 1, 0));
                        model = glm::scale(model, glm::vec3(2.0f, 1.0f, 5.0f));
                        model = model * transform3D::RotateOX(RADIANS(90.0f));
                        setUniforms(model);
                        meshes["map_train"]->Render();
                    }

                    if (meshes["map_arrow"])
                    {
                        glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(camera->position.x, 12.0f, camera->position.z));
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

            // HUD timer
            if (textRenderer)
            {
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

                float textWidth = 0.0f;
                for (char c : text)
                {
                    auto it = textRenderer->Characters.find(c);
                    if (it == textRenderer->Characters.end())
                        continue;
                    textWidth += (it->second.Advance >> 6) * scale;
                }
                textRenderer->RenderText(text, res.x - minimapMargin - textWidth, minimapMargin, scale, hudTimerColor);
            }
        }
    }

    void Tema2::OnInputUpdate(float deltaTime, int mods)
    {
        if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
        {
            float cameraSpeed = 2.5f;
            if (window->KeyHold(GLFW_KEY_LEFT_SHIFT))
                cameraSpeed *= 4.0f;
            if (window->KeyHold(GLFW_KEY_W))
                camera->TranslateForward(cameraSpeed * deltaTime);
            if (window->KeyHold(GLFW_KEY_A))
                camera->TranslateRight(-cameraSpeed * deltaTime);
            if (window->KeyHold(GLFW_KEY_S))
                camera->TranslateForward(-cameraSpeed * deltaTime);
            if (window->KeyHold(GLFW_KEY_D))
                camera->TranslateRight(cameraSpeed * deltaTime);
            if (window->KeyHold(GLFW_KEY_Q))
                camera->TranslateUpward(-cameraSpeed * deltaTime);
            if (window->KeyHold(GLFW_KEY_E))
                camera->TranslateUpward(cameraSpeed * deltaTime);
        }

        const float accel = 4.0f;
        const float minSpeed = 0.1f;
        const float maxSpeed = 20.0f;
        if (window->KeyHold(GLFW_KEY_UP))
            train.speed = glm::clamp(train.speed + accel * deltaTime, minSpeed, maxSpeed);
        if (window->KeyHold(GLFW_KEY_DOWN))
            train.speed = glm::clamp(train.speed - accel * deltaTime, minSpeed, maxSpeed);
    }

    void Tema2::OnKeyPress(int key, int mods)
    {
        if (train.waitingAtIntersection && cameraCentered && train.rail)
        {
            auto pickDirection = [&](const glm::vec3 &desired, bool rotate) -> bool
            {
                Rail *best = nullptr;
                float bestDot = 0.3f;
                glm::vec3 desiredDir = glm::normalize(desired);

                for (Rail *candidate : train.rail->nextRails)
                {
                    glm::vec3 candDir = glm::normalize(candidate->direction);
                    float d = glm::dot(desiredDir, candDir);
                    if (d > bestDot)
                    {
                        bestDot = d;
                        best = candidate;
                    }
                }

                if (best)
                {
                    // Save the current rail as "previous"
                    g_prevRail = train.rail;

                    train.rail = best;
                    // Reset progress close to 0 to start animation on the new rail
                    train.progress = 0.0f;

                    // The locomotive jumps to the beginning of the new rail, but the wagons will be drawn on g_prevRail
                    train.position = train.rail->startPosition + train.rail->direction * kStopOffset;
                    train.direction = train.rail->direction;
                    train.yaw = DirToYaw(train.direction);
                    train.waitingAtIntersection = false;
                    return true;
                }
                return false;
            };

            glm::vec3 forward = glm::normalize(train.rail->direction);
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

        if (key == GLFW_KEY_C)
        {
            cameraCentered = !cameraCentered;
            if (cameraCentered)
            {
                glm::vec3 locoPos = train.position + glm::vec3(0.0f, 0.6f, 0.0f);
                centeredOffset = glm::vec3(0.0f, 6.0f, 8.0f);
                if (camera)
                {
                    camera->Set(locoPos + centeredOffset, locoPos, glm::vec3(0.0f, 1.0f, 0.0f));
                }
            }
            else if (camera)
            {
                glm::vec3 pos = camera->position;
                glm::vec3 fwd = camera->forward;
                camera->Set(pos, pos + fwd, camera->up);
            }
        }

        if (key == GLFW_KEY_END)
        {
            endScreenActive = true;
            timer = 0.0f;
        }
    }

    void Tema2::OnKeyRelease(int key, int mods) {}

    void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
    {
        if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
        {
            float sensivityOX = 0.001f;
            float sensivityOY = 0.001f;

            if (cameraCentered)
            {
                float angleYaw = -sensivityOX * deltaX;
                centeredOffset = glm::vec3(glm::rotate(glm::mat4(1.0f), angleYaw, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(centeredOffset, 1.0f));

                glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
                glm::vec3 dir = glm::normalize(centeredOffset);
                glm::vec3 rightAxis = glm::normalize(glm::cross(dir, worldUp));
                float anglePitch = -sensivityOY * deltaY;
                if (glm::length(rightAxis) > 0.0001f)
                    centeredOffset = glm::vec3(glm::rotate(glm::mat4(1.0f), anglePitch, rightAxis) * glm::vec4(centeredOffset, 1.0f));
            }
            else
            {
                camera->RotateFirstPerson_OX(-sensivityOY * deltaY);
                camera->RotateFirstPerson_OY(-sensivityOX * deltaX);
            }
        }
    }

    void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) {}
    void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) {}

    void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
    {
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
                }
            }
        }
    }

    void Tema2::OnWindowResize(int width, int height)
    {
        if (height > 0)
            aspect = static_cast<float>(width) / static_cast<float>(height);
        else if (window && std::abs(window->props.aspectRatio) > std::numeric_limits<float>::epsilon())
            aspect = window->props.aspectRatio;
        if (std::abs(aspect) <= std::numeric_limits<float>::epsilon())
            aspect = 1.0f;

        projectionMatrix = glm::perspective(fovY, aspect, zNear, zFar);

        glm::ivec2 res = window ? window->GetResolution() : glm::ivec2(width, height);
        delete textRenderer;
        textRenderer = new gfxc::TextRenderer(window->props.selfDir, res.x, res.y);
        textRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, fontPath), 64);
    }
}
