#pragma once

#include "Material.h"
#include "Mesh.h"

#include <glm/glm.hpp>

#include <vector>

struct Sphere {
	glm::vec3 Position = glm::vec3(0.0f);
	float Radius = 0.5f;
	uint32_t MaterialIndex = 0;
};

struct Scene {
	std::vector<Material> Materials;
	std::vector<Sphere> Spheres;
	std::vector<Mesh> Meshes;
};