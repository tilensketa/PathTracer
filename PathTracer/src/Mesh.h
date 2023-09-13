#pragma once

#include "Vertex.h"
#include "AABB.h"

#include <glm/glm.hpp>

#include <vector>
#include <iostream>
#include <string>

struct Triangle {
	Vertex A;
	Vertex B;
	Vertex C;
};


class Mesh {
public:
	Mesh(std::string const& path, const glm::vec3& position);
	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, uint32_t materialIndex);

	const uint32_t& GetMaterialIndex() const { return m_MaterialIndex; }
	void SetMaterialIndex(uint32_t i);

	const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
	const std::vector<uint32_t>& GetIndices() const { return m_Indices; }
	const std::vector<Triangle>& GetTriangles() const { return m_Triangles; }

	const AABB& GetAABB() const { return m_AABB; }

	glm::vec3& Position() { return m_Position; }
	float& Scale() { return m_Scale; }
private:
	std::vector<Triangle> CalculateTriangles(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	AABB CreateAABB();
private:
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	std::vector<Triangle> m_Triangles;

	glm::vec3 m_Position = glm::vec3(0.0f, 0.0f, 0.0f);
	float m_Scale = 1.0f;
	uint32_t m_MaterialIndex = 0;

	AABB m_AABB;
};