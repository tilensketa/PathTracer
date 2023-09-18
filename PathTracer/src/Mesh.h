#pragma once

#include "Triangle.h"
#include "AABB.h"
#include "Material.h"

#include <glm/glm.hpp>

#include <vector>
#include <string>

class Mesh {
public:
	Mesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const Material& material);

	const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
	const std::vector<uint32_t>& GetIndices() const { return m_Indices; }
	const std::vector<Triangle>& GetTriangles() const { return m_Triangles; }

	const std::string& GetName() const { return m_Name; }
	const AABB& GetAABB() const { return m_AABB; }

	const Material& GetMaterial() const { return m_Material; }
	Material& GetMaterial() { return m_Material; }
private:
	void CalculateTriangles();
	AABB CreateAABB();
private:
	std::string m_Name;

	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	std::vector<Triangle> m_Triangles;

	AABB m_AABB;

	Material m_Material;
};