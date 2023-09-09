#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Material {
	glm::vec3 Albedo = glm::vec3(1.0f);
	float Roughness = 1.0f;
	float Metallic = 0.0f;
};

struct Sphere {
	glm::vec3 Position = glm::vec3(0.0f);
	float Radius = 0.5f;
	uint32_t MaterialIndex = 0;
};

struct Scene {
	std::vector<Material> Materials;
	std::vector<Sphere> Spheres;
};