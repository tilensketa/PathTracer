#pragma once

#include <glm/glm.hpp>

#include <vector>

struct BoundingTriangle {
	glm::vec3 A;
	glm::vec3 B;
	glm::vec3 C;
	glm::vec3 N;

	BoundingTriangle(const glm::vec3 a, const glm::vec3 b, const glm::vec3 c, const glm::vec3 n)
		: A(a), B(b), C(c), N(n) {}
};

class BoundingBox {
public:
	BoundingBox() = default;
	BoundingBox(const glm::vec3& maxCorner, const glm::vec3& minCorner);

	const std::vector<BoundingTriangle>& GetTriangles() const { return m_Triangles; }

private:
	std::vector<glm::vec3> m_Corners;
	std::vector<glm::vec3> m_Normals;
	std::vector<BoundingTriangle> m_Triangles;
};