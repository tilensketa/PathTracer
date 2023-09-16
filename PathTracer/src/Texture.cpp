#include "Texture.h"
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

Texture::Texture(const char* path) {
	m_ImageData = stbi_load(path, &m_Width, &m_Height, &m_Channels, 0);
	if (m_ImageData == nullptr) {
		spdlog::error("Failed to load texture: {}", path);
		return;
	}
	//spdlog::info("Texture({}) succesfully loaded", path);
}

glm::vec3 Texture::SampleTexture(const glm::vec2& texCoord) {
	int x = static_cast<int>(texCoord.x * m_Width);
	int y = static_cast<int>(texCoord.y * m_Height);
	uint32_t pixelIndex = (x + m_Width * y) * m_Channels;
	unsigned char red = m_ImageData[pixelIndex];
	unsigned char green = m_ImageData[pixelIndex+1];
	unsigned char blue = m_ImageData[pixelIndex+2];
	float r = static_cast<float>(red) / 255.0f;
	float g = static_cast<float>(green) / 255.0f;
	float b = static_cast<float>(blue) / 255.0f;
	return glm::vec3(r, g, b);
}