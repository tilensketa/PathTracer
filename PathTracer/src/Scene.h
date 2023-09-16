#pragma once

#include "Material.h"
#include "Texture.h"
#include "Model.h"

#include <glm/glm.hpp>

#include <vector>

struct Scene {
	std::vector<Model> Models;
};