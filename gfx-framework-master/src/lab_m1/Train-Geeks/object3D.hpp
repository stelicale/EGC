#pragma once

#include <string>

#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"

namespace object3D
{
    Mesh *CreateCircle(const std::string &name, glm::vec3 color);

    Mesh *CreateTriangle(const std::string& name, glm::vec3 color);

    Mesh *CreateSquare(const std::string &name, glm::vec3 color);

    Mesh *CreateStar(const std::string& name, glm::vec3 color);

    Mesh *CreateArrow(const std::string& name, glm::vec3 color);

    Mesh *CreateGround(
        const std::string &name,
        float length,
        glm::vec3 color);

    Mesh *CreateWater(
        const std::string &name,
        float width, float height);

    Mesh *CreateMountain(
        const std::string &name,
        float width, float height);

    Mesh *CreateTree(
        const std::string &name,
        glm::vec3 trunkColor,
        glm::vec3 leavesColor);

    Mesh *CreateLocomotive(
        const std::string &name,
        glm::vec3 bodyColor,
        glm::vec3 cabinColor,
        glm::vec3 wheelColor);

    Mesh *CreateWagon(
        const std::string &name,
        glm::vec3 bodyColor,
        glm::vec3 wheelColor,
        glm::vec3 supportColor);

    Mesh *CreateBasicTrainStation(
        const std::string &name,
        glm::vec3 platformColor,
        glm::vec3 wallColor,
        glm::vec3 doorColor,
        glm::vec3 symbolColor,
        glm::vec3 roofColor,
        const std::string &symbolShape);

    Mesh *CreateMainTrainStation(
        const std::string &name,
        glm::vec3 buildingColor,
        glm::vec3 roofColor,
        glm::vec3 platformColor);

    Mesh *CreateNormalRail(
        const std::string &name,
        glm::vec3 railColor,
        float length = 10.0f);

    Mesh *CreateTunnelRail(
        const std::string &name,
        glm::vec3 railColor,
        glm::vec3 tunnelColor,
        float length = 10.0f);

    Mesh *CreateBridgeRail(
        const std::string &name,
        glm::vec3 railColor,
        glm::vec3 bridgeColor,
        float length = 10.0f);

    Mesh *CreateCube(
        const std::string &name,
        glm::vec3 center,
        float size,
        glm::vec3 color,
        bool hasBlackBorder = true);

    Mesh *CreatePyramidMesh(
        const std::string &name,
        glm::vec3 baseCenter,
        float baseSize,
        float height,
        glm::vec3 color,
        bool hasBlackBorder = true);

    Mesh *CreateSphere(
        const std::string &name,
        glm::vec3 center,
        float radius,
        glm::vec3 color,
        bool hasBlackBorder = true);

    // Intersection rails - only draw rails on edges where connections exist
    // Directions: North (+Z), South (-Z), East (+X), West (-X)
    Mesh *CreateNormalRailIntersection(
        const std::string &name,
        glm::vec3 railColor,
        bool hasNorth, bool hasSouth, bool hasEast, bool hasWest);

    Mesh *CreateTunnelRailIntersection(
        const std::string &name,
        glm::vec3 railColor,
        glm::vec3 tunnelColor,
        bool hasNorth, bool hasSouth, bool hasEast, bool hasWest);

    Mesh *CreateBridgeRailIntersection(
        const std::string &name,
        glm::vec3 railColor,
        glm::vec3 bridgeColor,
        bool hasNorth, bool hasSouth, bool hasEast, bool hasWest);
}