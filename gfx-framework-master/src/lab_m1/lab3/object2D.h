#pragma once

#include <string>

#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"

namespace object2D
{
    // Create square with given bottom left corner, length and color
    Mesh* CreateSquare(const std::string &name, glm::vec3 leftBottomCorner, float length, glm::vec3 color, bool fill = false);
    
	// Create rectangle with given bottom left corner, width, height and color
	Mesh* CreateRectangle(const std::string& name, glm::vec3 leftBottomCorner, float width, float height, glm::vec3 color, bool fill = false);

    // Create circle with given center, radius and color
	Mesh* CreateCircle(const std::string& name, glm::vec3 center, float radius, glm::vec3 color, bool fill = false, int numSegments = 100);

	// Create bumper (semicircle on top of a square) with given center, radius, square size and colors
	Mesh* CreateBumper(const std::string& name, glm::vec3 center, float radius, float squareSize, glm::vec3 semiColor, glm::vec3 squareColor, bool fill = false, int numSegments = 100);

	// Create cannon (checkerboard square) with given bottom left corner, size and colors
	Mesh* CreateCannon(const std::string& name, glm::vec3 baseCorner, float squareSize, glm::vec3 blackColor, glm::vec3 whiteColor, bool fill = false);
}
