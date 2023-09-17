#include "Texture.h"

#include <spdlog/spdlog.h>

Texture::Texture(const char* path) {
	m_ImageData = stbi_load(path, &m_Width, &m_Height, &m_Channels, 0);
	if (m_ImageData == nullptr) {
		spdlog::error("Failed to load texture: {}", path);
		return;
	}

	// Find the position of the last directory separator
	const char* lastSeparator = strrchr(path, '/'); // Use '/' for Unix-like systems

	if (!lastSeparator) {
		lastSeparator = strrchr(path, '\\'); // Use '\\' for Windows
	}
	if (!lastSeparator) {
		m_Name = path; // No directory separator found, return the whole path as the filename
	}

	// Increment the pointer to get the filename (skip the separator)
	m_Name = lastSeparator + 1;

	spdlog::info("Texture({}) succesfully loaded", path);
}

const glm::vec3 Texture::SampleTexture(const glm::vec2& texCoord) const {
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

const glm::vec3 Texture::SampleSphericalTexture(const float& phi, const float& theta) const {
#define PI 3.1415926536
	// Map spherical coordinates to image coordinates
	float u = (phi + PI) / (2 * PI);  // Normalize azimuth angle to [0, 1]
	float v = (theta / PI);      // Normalize zenith angle to [0, 1]

	// Map to pixel coordinates
	//int x = static_cast<int>(u * (m_Width - 1));
	//int y = static_cast<int>(v * (m_Height - 1));

	// Sample the HDRI image
	return SampleTexture(glm::vec2(u, v));
}