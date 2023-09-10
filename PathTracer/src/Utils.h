#pragma once

#include <glm/glm.hpp>

namespace Random {

	float Float(float min, float max) {
		float r = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
		return r;
	}

	glm::vec3 Vec3(float min, float max) {
		float r2 = Float(min, max);
		float r3 = Float(min, max);
		float r1 = Float(min, max);
		return glm::vec3(r1, r2, r3);
	}

	glm::vec3 InUnitSphere() {
		return glm::normalize(Vec3(-1.0f, 1.0f));
	}

}