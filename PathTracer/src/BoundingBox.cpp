#include "BoundingBox.h"

BoundingBox::BoundingBox(const glm::vec3& maxCorner, const glm::vec3& minCorner) {

	m_Corners = {
		glm::vec3(maxCorner.x, maxCorner.y, minCorner.z),
		glm::vec3(maxCorner.x, minCorner.y, minCorner.z),
		glm::vec3(maxCorner.x, maxCorner.y, maxCorner.z),
		glm::vec3(maxCorner.x, minCorner.y, maxCorner.z),
		glm::vec3(minCorner.x, maxCorner.y, minCorner.z),
		glm::vec3(minCorner.x, minCorner.y, minCorner.z),
		glm::vec3(minCorner.x, maxCorner.y, maxCorner.z),
		glm::vec3(minCorner.x, minCorner.y, maxCorner.z),
	};
	m_Normals = {
		glm::vec3(0.0f,  1.0f,  0.0f),
		glm::vec3(0.0f,  0.0f,  1.0f),
		glm::vec3(-1.0f,  0.0f,  0.0f),
		glm::vec3(0.0f, -1.0f,  0.0f),
		glm::vec3(1.0f,  0.0f,  0.0f),
		glm::vec3(0.0f,  0.0f, -1.0f),
	};

	m_Triangles = {
		BoundingTriangle(m_Corners[0], m_Corners[4], m_Corners[6], m_Normals[0]),
		BoundingTriangle(m_Corners[0], m_Corners[6], m_Corners[2], m_Normals[0]),
		BoundingTriangle(m_Corners[3], m_Corners[2], m_Corners[6], m_Normals[1]),
		BoundingTriangle(m_Corners[3], m_Corners[6], m_Corners[7], m_Normals[1]),
		BoundingTriangle(m_Corners[7], m_Corners[6], m_Corners[4], m_Normals[2]),
		BoundingTriangle(m_Corners[7], m_Corners[4], m_Corners[5], m_Normals[2]),
		BoundingTriangle(m_Corners[5], m_Corners[1], m_Corners[3], m_Normals[3]),
		BoundingTriangle(m_Corners[5], m_Corners[3], m_Corners[7], m_Normals[3]),
		BoundingTriangle(m_Corners[1], m_Corners[0], m_Corners[2], m_Normals[4]),
		BoundingTriangle(m_Corners[1], m_Corners[2], m_Corners[3], m_Normals[4]),
		BoundingTriangle(m_Corners[5], m_Corners[4], m_Corners[0], m_Normals[5]),
		BoundingTriangle(m_Corners[5], m_Corners[0], m_Corners[1], m_Normals[5]),
	};
}