#pragma once

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoord;

	friend std::ostream& operator<<(std::ostream& os, const Vertex& vertex)
	{
		return os << vertex.Position.x << ", " << vertex.Position.y << ", " << vertex.Position.z;
	}
};