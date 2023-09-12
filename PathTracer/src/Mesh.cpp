#include "Mesh.h"
#include "OBJ_Loader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, uint32_t materialIndex)
	: m_Vertices(vertices), m_Indices(indices), m_MaterialIndex(materialIndex) {
	m_Triangles = CalculateTriangles(m_Vertices, m_Indices);
	m_BoundingBox = CreateBoundingBox();
}

Mesh::Mesh(std::string const& path, const glm::vec3& position) {

	m_Position = position;

	objl::Loader Loader;
	bool loadout = Loader.LoadFile(path);

	if (loadout)
	{
		for (int i = 0; i < Loader.LoadedMeshes.size(); i++)
		{
			const objl::Mesh& curMesh = Loader.LoadedMeshes[i];

			for (int j = 0; j < curMesh.Vertices.size(); j++)
			{
				auto vert = curMesh.Vertices[j];
				Vertex vertex;
				vertex.Position = glm::vec3(vert.Position.X, vert.Position.Y, vert.Position.Z) + m_Position;
				vertex.Normal = glm::vec3(vert.Normal.X, vert.Normal.Y, vert.Normal.Z);
				//vertex.TexCoord = glm::vec2(vert.TextureCoordinate.X, vert.TextureCoordinate.Y);
				m_Vertices.push_back(vertex);
			}
			for (uint32_t j = 0; j < curMesh.Indices.size(); j++)
			{
				m_Indices.push_back(curMesh.Indices[j]);
			}

			m_Triangles = CalculateTriangles(m_Vertices, m_Indices);
			m_BoundingBox = CreateBoundingBox();
		}
	}
	else {
		spdlog::error("Failed to load mesh");
	}
}

void Mesh::SetMaterialIndex(uint32_t i) {
	m_MaterialIndex = i;
}

std::vector<Triangle> Mesh::CalculateTriangles(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
	std::vector<Triangle> result;
	for (uint32_t i = 0; i < indices.size(); i += 3)
	{
		Triangle triangle;
		triangle.A = vertices[indices[i]];
		triangle.B = vertices[indices[i + 1]];
		triangle.C = vertices[indices[i + 2]];
		result.push_back(triangle);
	}
	return result;
}

BoundingBox Mesh::CreateBoundingBox() {
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
	return BoundingBox(maxVert, minVert);
}