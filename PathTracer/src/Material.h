#pragma once

#include "Texture.h"

#include <glm/glm.hpp>

#include <spdlog/spdlog.h>

#include <vector>


struct Material {
	std::string Name = "NONAME";
	glm::vec3 AmbientColor = glm::vec3(1.0f);
	glm::vec3 DiffuseColor = glm::vec3(1.0f);
	glm::vec3 SpecularColor = glm::vec3(1.0f);
	glm::vec3 EmissionColor = glm::vec3(0.0f);
	glm::vec3 ReflectiveColor = glm::vec3(1.0f);
	glm::vec3 TransparentColor = glm::vec3(1.0f);
	float Metallic = 0.0f;
	float Shininess = 0.0f;
	float Roughness = 1.0f;
	float Specular = 0.0f;
	float EmissionPower = 1.0f;

	std::vector<Texture> Textures;
	int DiffuseTextureIndex = -1;
	int SpecularTextureIndex = -1;
	int NormalTextureIndex = -1;
	int ShininessTextureIndex = -1;

	glm::vec3 GetEmission() const { return EmissionColor * EmissionPower; }

	friend std::ostream& operator<<(std::ostream& os, const Material& material)
	{
		return os << "Diffuse color (RGB): " << material.DiffuseColor.r << ", " << material.DiffuseColor.g << ", " << material.DiffuseColor.b;
	}
};