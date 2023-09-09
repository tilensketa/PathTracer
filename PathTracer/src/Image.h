#pragma once

#include <glm/glm.hpp>

#include <vector>

class Image {
public:
	Image(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height) {
		m_Pixels.resize(width * height);
	}

	void Clear() {
		m_Pixels.clear();
		m_Pixels.resize(m_Width * m_Height);
	}

	void OnResize(uint32_t width, uint32_t height) {
		if (m_Width == width && m_Height == height)
			return;

		m_Width = width;
		m_Height = height;
		m_Pixels.resize(m_Width * m_Height);
	}

	void SetPixel(const uint32_t& x, const uint32_t& y, const glm::vec3& color) {
		uint32_t i = y * m_Width + x;
		m_Pixels[i] = color;
	}

	const uint32_t& GetWidth() const { return m_Width; }
	const uint32_t& GetHeight() const { return m_Height; }
	const std::vector<glm::vec3>& GetPixels() const { return m_Pixels; }
	const glm::vec3& GetPixel(uint32_t i) const { return m_Pixels[i]; }

private:
	std::vector<glm::vec3> m_Pixels;
	uint32_t m_Width;
	uint32_t m_Height;
};