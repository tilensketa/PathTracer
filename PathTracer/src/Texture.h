#pragma once

#include <glm/glm.hpp>

#include <stb/stb_image.h>

class Texture {
public:
	Texture(const char* path);

	glm::vec3 SampleTexture(const glm::vec2& texCoord);

	const int& GetWidth() const { return m_Width; }
	const int& GetHeight() const { return m_Height; }
	const unsigned char* GetData() const { return m_ImageData; }
private:
	int m_Width, m_Height;
	int m_Channels;
	unsigned char* m_ImageData;
};