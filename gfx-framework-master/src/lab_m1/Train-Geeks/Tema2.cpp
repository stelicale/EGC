#include "lab_m1/Train-Geeks/Tema2.hpp"
#include "lab_m1/Train-Geeks/object3D.cpp"
#include <algorithm>
#include <cmath>

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
        // Text renderer for ending screen
        glm::ivec2 resolution = window ? window->GetResolution() : glm::ivec2(1280, 720);
        delete textRenderer;
        textRenderer = new gfxc::TextRenderer(window->props.selfDir, resolution.x, resolution.y);
        textRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, fontPath), 64);

        camera = new implemented::Camera();
        camera->Set(glm::vec3(0, 3, 7), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
        // Compute a safe aspect ratio (avoid zero / near-epsilon values that trigger GLM asserts)
        float aspectInit = 16.0f / 9.0f; // default fallback
        if (window)
        {
            const auto &res = window->props.resolution;
            if (res.y != 0)
            {
                aspectInit = static_cast<float>(res.x) / static_cast<float>(res.y);
            }
            else if (std::abs(window->props.aspectRatio) > std::numeric_limits<float>::epsilon())
            {
                aspectInit = window->props.aspectRatio;
            }
        }
        if (std::abs(aspectInit) <= std::numeric_limits<float>::epsilon())
            aspectInit = 1.0f;

        projectionMatrix = glm::perspective(fovY, aspectInit, zNear, zFar);

        // initialize camera toggle saved state
        cameraCentered = false;
        if (camera)
        {
            // in viitor vreau ca pozitia (si nu numai) sa fie cea care corespunde locomotivei in miscare
            savedCamPos = camera->position;
            savedCamForward = camera->forward;
            savedCamUp = camera->up;
            savedDistanceToTarget = camera->distanceToTarget;
            // initial centered offset (relative to locomotive at origin)
            glm::vec3 locoPosInit = glm::vec3(0.0f, 0.6f, 0.0f);
            centeredOffset = savedCamPos - locoPosInit;
        }

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

        // Build bidirectional rail network as linked rails between intersections (simple local storage)
        static std::vector<Rail> rails;
        rails.clear();
        rails.reserve(32);

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
            rail.dir = (rail.length > 0.0f) ? delta / rail.length : glm::vec3(0.0f, 0.0f, 0.0f);
            return &rail;
        };

        auto addPair = [&](const glm::vec3 &a, const glm::vec3 &b)
        {
            makeRail(a, b);
            makeRail(b, a);
        };

        // Intersections (x,z): (-20, -20/0/20), (2.5, -20/0/20), (38, -20/0/20)
        glm::vec3 i1(-20.0f, 0.0f, -20.0f);
        glm::vec3 i2(-20.0f, 0.0f, 0.0f);
        glm::vec3 i3(-20.0f, 0.0f, 20.0f);
        glm::vec3 i4(2.5f, 0.0f, -20.0f);
        glm::vec3 i5(2.5f, 0.0f, 0.0f);
        glm::vec3 i6(2.5f, 0.0f, 20.0f);
        glm::vec3 i7(38.0f, 0.0f, -20.0f);
        glm::vec3 i8(38.0f, 0.0f, 0.0f);
        glm::vec3 i9(38.0f, 0.0f, 20.0f);

        // Vertical segments
        addPair(i1, i2);
        addPair(i2, i3);
        addPair(i4, i5);
        addPair(i5, i6);
        addPair(i7, i8);
        addPair(i8, i9);
        // Horizontal segments
        addPair(i1, i4);
        addPair(i4, i7);
        addPair(i2, i5);
        addPair(i5, i8);
        addPair(i3, i6);
        addPair(i6, i9);

        // Link next rails automatically: any rail ending at a node connects to rails starting at that node
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

        // Initialize train on the mountain-area intersection along z = 0 (i2 -> i5)
        Rail *startRail = nullptr;
        for (auto &r : rails)
        {
            if (closeEnough(r.startPosition, i2) && closeEnough(r.endPosition, i5))
            {
                startRail = &r;
                break;
            }
        }

        // Fallback to first rail if the specific one is not found
        train.rail = startRail ? startRail : (rails.empty() ? nullptr : &rails.front());
        train.progress = 0.0f;
        train.speed = 4.0f;
        train.position = train.rail ? train.rail->startPosition : glm::vec3(0.0f);
        train.direction = train.rail ? train.rail->dir : glm::vec3(1.0f, 0.0f, 0.0f);
        train.yaw = DirToYaw(train.direction);
        train.numWagons = 2;
        train.waitingAtIntersection = false;
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
        std::string meshName = getRailType(pos);

        if (meshes.count(meshName))
        {
            glm::mat4 model = glm::mat4(1);
            model = glm::translate(model, pos);
            if (rotate90)
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
                const std::string msg = "The End";
                // Scale text with resolution height (cap to reasonable limits)
                float scale = glm::clamp(std::min(res.x, res.y) / 720.0f, 0.5f, 3.0f);

                // Measure text width/height using glyph metrics for accurate centering
                float textWidth = 0.0f;
                float maxSizeY = 0.0f;
                for (char c : msg)
                {
                    auto it = textRenderer->Characters.find(c);
                    if (it == textRenderer->Characters.end())
                        continue;
                    const auto &ch = it->second;
                    textWidth += (ch.Advance >> 6) * scale; // Advance is in 1/64 pixels
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

        // If camera is centered on locomotive, update it to follow the locomotive
        if (cameraCentered && camera)
        {
            // compute camera position from the stored offset (allows orbiting)
            glm::vec3 camPos = train.position + centeredOffset;
            camera->Set(camPos, train.position, glm::vec3(0.0f, 1.0f, 0.0f));
            camera->distanceToTarget = glm::length(centeredOffset);
        }

        // accumulate HUD timer when game is running
        elapsedTime += deltaTimeSeconds;

        // std::cout << getRailType(camera->GetTargetPosition()) << std::endl;

        glm::mat4 modelMatrix = glm::mat4(1);

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

        modelMatrix = glm::mat4(1);
        RenderMesh(meshes["ground"], shaders["VertexColor"], glm::mat4(1));

        modelMatrix = transform3D::Translate(30.0f, 0.0f, 0.0f);
        RenderMesh(meshes["water"], shaders["VertexColor"], modelMatrix);

        modelMatrix = glm::mat4(1);
        modelMatrix = transform3D::Translate(-15.0f, 0.0f, 0.0f);
        RenderMesh(meshes["mountain"], shaders["VertexColor"], modelMatrix);

        // Advance train along current rail
        if (train.rail && !train.waitingAtIntersection)
        {
            float safeLength = std::max(train.rail->length - 2.0f * kStopOffset, 0.0001f);
            glm::vec3 startMove = train.rail->startPosition + train.rail->dir * kStopOffset;
            glm::vec3 endStop = train.rail->endPosition - train.rail->dir * kStopOffset;

            train.progress += (train.speed * deltaTimeSeconds) / safeLength;

            if (train.progress >= 1.0f)
            {
                train.progress = 1.0f;
                train.position = endStop;
                train.direction = train.rail->dir;
                train.yaw = DirToYaw(train.direction);

                const auto &choices = train.rail->nextRails;
                if (choices.empty())
                {
                    train.waitingAtIntersection = true;
                }
                else if (choices.size() == 1)
                {
                    // Continue from the same offset distance on the next rail for a smooth handoff
                    train.rail = choices[0];
                    float nextSafeLength = std::max(train.rail->length - 2.0f * kStopOffset, 0.0001f);
                    glm::vec3 nextStartMove = train.rail->startPosition + train.rail->dir * kStopOffset;
                    float proj = glm::dot(train.position - nextStartMove, train.rail->dir);
                    train.progress = proj / nextSafeLength; // may start slightly negative; Update will advance smoothly
                    train.position = train.position;        // keep current spot to avoid visual jump
                    train.direction = train.rail->dir;
                    train.yaw = DirToYaw(train.direction);
                }
                else
                {
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

        // Render locomotive and wagons using current train state
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

        modelMatrix = glm::mat4(1);
        modelMatrix = transform3D::Translate(42.0f, 0.0f, -13.0f) * transform3D::RotateOY(RADIANS(-90.0f));
        RenderMesh(meshes["cubeStation"], shaders["VertexColor"], modelMatrix);

        modelMatrix = glm::mat4(1);
        modelMatrix = transform3D::Translate(7.5f, 0.0f, 17.5f);
        RenderMesh(meshes["pyramidStation"], shaders["VertexColor"], modelMatrix);

        modelMatrix = glm::mat4(1);
        modelMatrix = transform3D::Translate(-25.0f, 0.0f, -13.0f) * transform3D::RotateOY(RADIANS(90.0f));
        RenderMesh(meshes["sphereStation"], shaders["VertexColor"], modelMatrix);

        modelMatrix = glm::mat4(1);
        modelMatrix = transform3D::Translate(0.0f, 0.0f, 2.5f) * transform3D::RotateOY(RADIANS(180.0f));
        RenderMesh(meshes["mainStation"], shaders["VertexColor"], modelMatrix);

        float xLines[3] = {-20.0f, 2.5f, 38.0f};
        for (float xVal : xLines)
        {
            for (float z = -20.0f; z <= 20.0f; z += 2.0f)
            {
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
    }

    void Tema2::FrameEnd()
    {
        // Draw main scene coordinate system if needed
        // DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);

        // --- MINIMAP (bottom-right) ---
        if (window && !endScreenActive)
        {
            glm::ivec2 res = window->GetResolution();
            int size = std::min(res.x, res.y) / 3;
            if (size > 8)
            {
                int margin = minimapMargin;
                int vpX = res.x - margin - size;
                int vpY = margin; // bottom margin

                // Save previous viewport
                GLint prevViewport[4];
                glGetIntegerv(GL_VIEWPORT, prevViewport);

                // Set new viewport for minimap
                glViewport(vpX, vpY, size, size);

                // Prepare an orthographic projection covering minimapWorldSize centered on locomotive
                float halfWorld = minimapWorldSize * 0.5f;
                glm::mat4 projMinimap = glm::ortho(-halfWorld, halfWorld, -halfWorld, halfWorld, -100.0f, 100.0f);

                // Top-down view: center on the current 3D camera's XZ coordinates (projected onto X-OZ plane)
                glm::vec3 center;
                if (camera)
                {
                    center = glm::vec3(camera->position.x, 0.0f, camera->position.z);
                }
                else
                {
                    center = glm::vec3(0.0f, 0.0f, 0.0f); // fallback
                }
                glm::vec3 eye = center + glm::vec3(0.0f, minimapWorldSize, 0.0f);
                glm::vec3 up = glm::vec3(0.0f, 0.0f, -1.0f); // make -Z point upwards on minimap
                glm::mat4 viewMinimap = glm::lookAt(eye, center, up);

                // Use the same shader as main rendering (VertexColor)
                Shader *shader = shaders["VertexColor"];
                if (shader && shader->program)
                {
                    // Enable depth test for correct occlusion in minimap
                    GLboolean depthEnabled;
                    glGetBooleanv(GL_DEPTH_TEST, &depthEnabled);
                    glEnable(GL_DEPTH_TEST);

                    shader->Use();

                    auto setUniforms = [&](const glm::mat4 &model)
                    {
                        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMinimap));
                        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projMinimap));
                        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(model));
                    };

                    auto renderRailMini = [&](const glm::vec3 &pos, bool rotate90)
                    {
                        std::string meshName = getRailType(pos);
                        if (!meshes.count(meshName))
                            return;
                        glm::mat4 m = glm::translate(glm::mat4(1), pos);
                        if (rotate90)
                            m = m * transform3D::RotateOY(RADIANS(90.0f));
                        setUniforms(m);
                        meshes[meshName]->Render();
                    };

                    // ground
                    if (meshes.count("ground") && meshes["ground"])
                    {
                        setUniforms(glm::mat4(1));
                        meshes["ground"]->Render();
                    }

                    // water
                    if (meshes.count("water") && meshes["water"])
                    {
                        glm::mat4 waterModel = glm::translate(glm::mat4(1), glm::vec3(30.0f, 0.0f, 0.0f));
                        setUniforms(waterModel);
                        meshes["water"]->Render();
                    }

                    // mountain
                    if (meshes.count("mountain") && meshes["mountain"])
                    {
                        glm::mat4 mountainModel = glm::translate(glm::mat4(1), glm::vec3(-15.0f, 0.0f, 0.0f));
                        setUniforms(mountainModel);
                        meshes["mountain"]->Render();
                    }

                    // trees (mirror main scene placement/exclusions)
                    for (float z = -50.0f; z <= 50.0f; z += 5.0f)
                    {
                        for (float x = -50.0f; x <= 50.0f; x += 5.0f)
                        {
                            if (((z == 0.0f || z == 20.0f || z == -20.0f) && x >= -20.0f && x <= 38.0f) || ((x == -20.0f || x == 38.0f) && z >= -20.0f && z <= 20.0f))
                                continue;
                            glm::mat4 treeModel = transform3D::Translate(x, 0.0f, z);
                            setUniforms(treeModel);
                            meshes["tree"]->Render();
                        }
                    }

                    // rails grid (same as main scene)
                    float xLines[3] = {-20.0f, 2.5f, 38.0f};
                    for (float xVal : xLines)
                    {
                        for (float z = -20.0f; z <= 20.0f; z += 2.0f)
                        {
                            renderRailMini(glm::vec3(xVal, 0.0f, z), true);
                        }
                    }

                    float zLines[3] = {-20.0f, 0.0f, 20.0f};
                    for (float zVal : zLines)
                    {
                        for (float x = -20.0f; x <= 38.0f; x += 2.0f)
                        {
                            renderRailMini(glm::vec3(x, 0.0f, zVal), false);
                        }
                    }

                    // stations (match main scene transforms)
                    if (meshes.count("cubeStation") && meshes["cubeStation"])
                    {
                        glm::mat4 m = transform3D::Translate(42.0f, 0.0f, -13.0f) * transform3D::RotateOY(RADIANS(-90.0f));
                        setUniforms(m);
                        meshes["cubeStation"]->Render();
                    }
                    if (meshes.count("pyramidStation") && meshes["pyramidStation"])
                    {
                        glm::mat4 m = transform3D::Translate(7.5f, 0.0f, 17.5f);
                        setUniforms(m);
                        meshes["pyramidStation"]->Render();
                    }
                    if (meshes.count("sphereStation") && meshes["sphereStation"])
                    {
                        glm::mat4 m = transform3D::Translate(-25.0f, 0.0f, -13.0f) * transform3D::RotateOY(RADIANS(90.0f));
                        setUniforms(m);
                        meshes["sphereStation"]->Render();
                    }
                    if (meshes.count("mainStation") && meshes["mainStation"])
                    {
                        glm::mat4 m = transform3D::Translate(0.0f, 0.0f, 2.5f) * transform3D::RotateOY(RADIANS(180.0f));
                        setUniforms(m);
                        meshes["mainStation"]->Render();
                    }

                    // locomotive and wagons (use actual train position)
                    if (meshes.count("locomotive") && meshes["locomotive"])
                    {
                        glm::mat4 locoModel = transform3D::Translate(train.position.x, train.position.y, train.position.z) * transform3D::RotateOY(train.yaw);
                        setUniforms(locoModel);
                        meshes["locomotive"]->Render();
                    }
                    if (meshes.count("wagon") && meshes["wagon"])
                    {
                        for (int i = 0; i < train.numWagons; i++)
                        {
                            glm::vec3 wagonPos = train.position - train.direction * wagonLength * static_cast<float>(i + 1);
                            glm::mat4 wagonModel = transform3D::Translate(wagonPos.x, wagonPos.y, wagonPos.z) * transform3D::RotateOY(train.yaw);
                            setUniforms(wagonModel);
                            meshes["wagon"]->Render();
                        }
                    }

                    // restore depth test state
                    if (!depthEnabled)
                        glDisable(GL_DEPTH_TEST);
                }

                // Restore previous viewport
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
                float textHeight = maxSizeY * scale;

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
                        train.progress = 1 / nextSafeLength;
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
