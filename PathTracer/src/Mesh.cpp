#include "Mesh.h"

Mesh::Mesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const Material& material)
	: m_Name(name), m_Vertices(vertices), m_Indices(indices){
	CalculateTriangles();
	m_AABB = CreateAABB();
	m_Material = material;
}

void Mesh::CalculateTriangles() {
	for (uint32_t i = 0; i < m_Indices.size(); i += 3)
	{
		Triangle triangle;
		triangle.A = m_Vertices[m_Indices[i]];
		triangle.B = m_Vertices[m_Indices[i + 1]];
		triangle.C = m_Vertices[m_Indices[i + 2]];
		m_Triangles.push_back(triangle);
	}
}

AABB Mesh::CreateAABB() {
	float max = std::numeric_limits<float>::max();
	float min = std::numeric_limits<float>::min();
	glm::vec3 minVert = glm::vec3(max, max, max);
	glm::vec3 maxVert = glm::vec3(-max, -max, -max);
	for (uint32_t i = 0; i < m_Vertices.size(); i++)
	{
		const Vertex& vert = m_Vertices[i];
		minVert.x = std::min(minVert.x, vert.Position.x);
		minVert.y = std::min(minVert.y, vert.Position.y);
		minVert.z = std::min(minVert.z, vert.Position.z);

		maxVert.x = std::max(maxVert.x, vert.Position.x);
		maxVert.y = std::max(maxVert.y, vert.Position.y);
		maxVert.z = std::max(maxVert.z, vert.Position.z);
	}
	return AABB(maxVert, minVert);
}