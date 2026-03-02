#pragma once

#include <string>

#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"

namespace object3D
{
    /**
     * @brief Creates a square 2D object
     * 
     * @param name The name of the mesh
     * @param color The color of the mesh
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateCircle(const std::string &name, glm::vec3 color);

    /**
     * @brief Creates a triangle 2D object
     * 
     * @param name The name of the mesh
     * @param color The color of the mesh
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateTriangle(const std::string& name, glm::vec3 color);

    /**
     * @brief Creates a square 2D object
     * 
     * @param name The name of the mesh
     * @param color The color of the mesh
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateSquare(const std::string &name, glm::vec3 color);

    /**
     * @brief Creates a rectangle 2D object
     * 
     * @param name The name of the mesh
     * @param length The length of the rectangle
     * @param height The height of the rectangle
     * @param color The color of the mesh
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateRectangle(
        const std::string &name,
        float length,
        float height,
        glm::vec3 color);

    /**
     * @brief Creates a star 2D object
     * @param name The name of the mesh
     * @param color The color of the mesh
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateStar(const std::string& name, glm::vec3 color);

    /**
     * @brief Creates an arrow 2D object
     * @param name The name of the mesh
     * @param color The color of the mesh
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateArrow(const std::string& name, glm::vec3 color);

    /**
     * @brief Creates a circle 2D object with a hole in the middle
     * @param name The name of the mesh
     * @param outerRadius The outer radius of the circle
     * @param innerRadius The inner radius of the circle
     * @param color The color of the mesh
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateGround(
        const std::string &name,
        float length,
        glm::vec3 color);

    /**
     * @brief Creates a water 2D object
     * @param name The name of the mesh
     * @param width The width of the water
     * @param height The height of the water
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateWater(
        const std::string &name,
        float width, float height);

    /**
     * @brief Creates a mountain 2D object
     * @param name The name of the mesh
     * @param width The width of the mountain
     * @param height The height of the mountain
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateMountain(
        const std::string &name,
        float width, float height);

    /**
     * @brief Creates a tree 2D object
     * @param name The name of the mesh
     * @param trunkColor The color of the trunk
     * @param leavesColor The color of the leaves
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateTree(
        const std::string &name,
        glm::vec3 trunkColor,
        glm::vec3 leavesColor);

    /**
     * @brief Creates a locomotive 3D object
     * @param name The name of the mesh
     * @param bodyColor The color of the body
     * @param cabinColor The color of the cabin
     * @param wheelColor The color of the wheels
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateLocomotive(
        const std::string &name,
        glm::vec3 bodyColor,
        glm::vec3 cabinColor,
        glm::vec3 wheelColor);

    /**
     * @brief Creates a wagon 3D object
     * @param name The name of the mesh
     * @param bodyColor The color of the body
     * @param wheelColor The color of the wheels
     * @param supportColor The color of the supports
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateWagon(
        const std::string &name,
        glm::vec3 bodyColor,
        glm::vec3 wheelColor,
        glm::vec3 supportColor);

    /**
     * @brief Creates a basic train station 3D object
     * @param name The name of the mesh
     * @param platformColor The color of the platform
     * @param wallColor The color of the walls
     * @param doorColor The color of the doors
     * @param symbolColor The color of the symbols
     * @param roofColor The color of the roof
     * @param symbolShape The shape of the symbols (e.g., "circle", "square", "triangle")
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateBasicTrainStation(
        const std::string &name,
        glm::vec3 platformColor,
        glm::vec3 wallColor,
        glm::vec3 doorColor,
        glm::vec3 symbolColor,
        glm::vec3 roofColor,
        const std::string &symbolShape);

    /**
     * @brief Creates the main train station 3D object
     * @param name The name of the mesh
     * @param buildingColor The color of the building
     * @param roofColor The color of the roof
     * @param platformColor The color of the platform
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateMainTrainStation(
        const std::string &name,
        glm::vec3 buildingColor,
        glm::vec3 roofColor,
        glm::vec3 platformColor);

    /**
     * @brief Creates a rail 3D object
     * @param name The name of the mesh
     * @param railColor The color of the rail
     * @param length The length of the rail
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateNormalRail(
        const std::string &name,
        glm::vec3 railColor,
        float length = 10.0f);

    /**
     * @brief Creates a tunnel rail 3D object
     * @param name The name of the mesh
     * @param railColor The color of the rail
     * @param tunnelColor The color of the tunnel
     * @param length The length of the rail
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateTunnelRail(
        const std::string &name,
        glm::vec3 railColor,
        glm::vec3 tunnelColor,
        float length = 10.0f);

    /**
     * @brief Creates a bridge rail 3D object
     * @param name The name of the mesh
     * @param railColor The color of the rail
     * @param bridgeColor The color of the bridge
     * @param length The length of the rail
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateBridgeRail(
        const std::string &name,
        glm::vec3 railColor,
        glm::vec3 bridgeColor,
        float length = 10.0f);

    /**
     * @brief Creates a cube 3D object
     * @param name The name of the mesh
     * @param center The center position of the cube
     * @param size The size of the cube
     * @param color The color of the cube
     * @param hasBlackBorder Whether the cube has a black border
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateCube(
        const std::string &name,
        glm::vec3 center,
        float size,
        glm::vec3 color,
        bool hasBlackBorder = true);

    /**
     * @brief Creates a pyramid 3D object
     * @param name The name of the mesh
     * @param baseCenter The center position of the pyramid's base
     * @param baseSize The size of the pyramid's base
     * @param height The height of the pyramid
     * @param color The color of the pyramid
     * @param hasBlackBorder Whether the pyramid has a black border
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreatePyramidMesh(
        const std::string &name,
        glm::vec3 baseCenter,
        float baseSize,
        float height,
        glm::vec3 color,
        bool hasBlackBorder = true);

    /**
     * @brief Creates a sphere 3D object
     * @param name The name of the mesh
     * @param center The center position of the sphere
     * @param radius The radius of the sphere
     * @param color The color of the sphere
     * @param hasBlackBorder Whether the sphere has a black border
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateSphere(
        const std::string &name,
        glm::vec3 center,
        float radius,
        glm::vec3 color,
        bool hasBlackBorder = true);

    /**
     * @brief Creates a normal rail intersection 3D object
     * @param name The name of the mesh
     * @param railColor The color of the rail
     * @param hasNorth Whether there is a rail to the north
     * @param hasSouth Whether there is a rail to the south
     * @param hasEast Whether there is a rail to the east
     * @param hasWest Whether there is a rail to the west
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateNormalRailIntersection(
        const std::string &name,
        glm::vec3 railColor,
        bool hasNorth, bool hasSouth, bool hasEast, bool hasWest);

    /**
     * @brief Creates a tunnel rail intersection 3D object
     * @param name The name of the mesh
     * @param railColor The color of the rail
     * @param tunnelColor The color of the tunnel
     * @param hasNorth Whether there is a rail to the north
     * @param hasSouth Whether there is a rail to the south
     * @param hasEast Whether there is a rail to the east
     * @param hasWest Whether there is a rail to the west
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateTunnelRailIntersection(
        const std::string &name,
        glm::vec3 railColor,
        glm::vec3 tunnelColor,
        bool hasNorth, bool hasSouth, bool hasEast, bool hasWest);

    /**
     * @brief Creates a bridge rail intersection 3D object
     * @param name The name of the mesh
     * @param railColor The color of the rail
     * @param bridgeColor The color of the bridge
     * @param hasNorth Whether there is a rail to the north
     * @param hasSouth Whether there is a rail to the south
     * @param hasEast Whether there is a rail to the east
     * @param hasWest Whether there is a rail to the west
     * @return Mesh* Pointer to the created mesh
     */
    Mesh *CreateBridgeRailIntersection(
        const std::string &name,
        glm::vec3 railColor,
        glm::vec3 bridgeColor,
        bool hasNorth, bool hasSouth, bool hasEast, bool hasWest);
}