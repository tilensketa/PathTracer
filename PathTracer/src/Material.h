#pragma once

#include <glm/glm.hpp>

struct Material {
	glm::vec3 Albedo = glm::vec3(1.0f);
	float Roughness = 1.0f;
	float Metallic = 0.0f;
	glm::vec3 EmissionColor = glm::vec3(0.0f);
	float EmissionPower = 1.0f;

	glm::vec3 GetEmmision() const { return EmissionColor * EmissionPower; }
};