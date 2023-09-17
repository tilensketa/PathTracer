#pragma once

#include "Model.h"
#include "Texture.h"

#include <vector>

struct Scene {

	std::vector<Model> Models;
	std::vector<Texture> EnvironmentImages;
	float EnvironmetStrength = 1.0f;
	uint32_t SelectedEnvironment = 0;
	float EnvironmentRotation = 0;
};