#pragma once

#include <glm/glm.hpp>

namespace Random {

	glm::vec3 vec3(float min, float max) {
		float r1 = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
		float r2 = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
		float r3 = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
		return glm::vec3(r1, r2, r3);
	}

}