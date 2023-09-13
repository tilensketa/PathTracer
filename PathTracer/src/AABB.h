#pragma once

#include <glm/glm.hpp>

struct AABB {
	glm::vec3 Max;
	glm::vec3 Min;

	AABB() = default;
	AABB(const glm::vec3& max, const glm::vec3& min)
		: Max(max), Min(min) {}
};