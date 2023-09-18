#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>

struct AABB {
	glm::vec3 Max;
	glm::vec3 Min;

	AABB() = default;
	AABB(const glm::vec3& max, const glm::vec3& min)
		: Max(max), Min(min) {}

	bool Intersects(glm::vec3 origin, glm::vec3 direction) const {
		glm::vec3 invDir = 1.0f / direction;
		glm::vec3 t1 = (Min - origin) * invDir;
		glm::vec3 t2 = (Max - origin) * invDir;

		float tNear = glm::compMax(glm::min(t1, t2));
		float tFar = glm::compMin(glm::max(t1, t2));

		return tNear <= tFar && tFar >= 0;
	}
};