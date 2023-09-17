#pragma once

#include <glm/glm.hpp>

#include <stb/stb_image.h>

#include <string>

class Texture {
public:
	Texture(const char* path);

	const glm::vec3 SampleTexture(const glm::vec2& texCoord) const;
	const glm::vec3 SampleSphericalTexture(const float& phi, const float& theta) const;

	const int& GetWidth() const { return m_Width; }
	const int& GetHeight() const { return m_Height; }
	const unsigned char* GetData() const { return m_ImageData; }
	const std::string GetName() const { return m_Name; }
private:
	int m_Width, m_Height;
	int m_Channels;
	unsigned char* m_ImageData;
	std::string m_Name;
};