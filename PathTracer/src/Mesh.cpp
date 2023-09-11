#include "Mesh.h"
#include "OBJ_Loader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, uint32_t materialIndex)
	: m_Vertices(vertices), m_Indices(indices), m_MaterialIndex(materialIndex) {
	CalculateTriangles();
	m_MaterialIndex = 1;
}

Mesh::Mesh(std::string const& path, const glm::vec3& position) {
	m_Position = position;

	// Initialize Loader
	objl::Loader Loader;

	// Load .obj File
	bool loadout = Loader.LoadFile(path);

	// If so continue
	if (loadout)
	{
		//spdlog::info("Loading mesh");
		for (int i = 0; i < Loader.LoadedMeshes.size(); i++)
		{
			objl::Mesh curMesh = Loader.LoadedMeshes[i];

			//spdlog::info("Mesh {}: {}", i, curMesh.MeshName);

			//spdlog::info("Vertices");

			for (int j = 0; j < curMesh.Vertices.size(); j++)
			{
				auto vert = curMesh.Vertices[j];
				//spdlog::info("{}, {}, {}", vert.Position.X, vert.Position.Y, vert.Position.Z);
				Vertex vertex;
				vertex.Position = glm::vec3(vert.Position.X, vert.Position.Y, vert.Position.Z) + m_Position;
				vertex.Normal = glm::vec3(vert.Normal.X, vert.Normal.Y, vert.Normal.Z);
				vertex.TexCoord = glm::vec2(vert.TextureCoordinate.X, vert.TextureCoordinate.Y);
				m_Vertices.push_back(vertex);
			}

			//spdlog::info("Indices");

			for (int j = 0; j < curMesh.Indices.size(); j += 3)
			{
				//spdlog::info("{}, {}, {}", curMesh.Indices[j], curMesh.Indices[j+1], curMesh.Indices[j+2]);
				m_Indices.push_back(curMesh.Indices[j]);
				m_Indices.push_back(curMesh.Indices[j+1]);
				m_Indices.push_back(curMesh.Indices[j+2]);
			}

			//spdlog::info("Material: {}", curMesh.MeshMaterial.name);
			//spdlog::info("Ambient Color: {}, {}, {}", curMesh.MeshMaterial.Ka.X, curMesh.MeshMaterial.Ka.Y, curMesh.MeshMaterial.Ka.Z);

			CalculateTriangles();
		}
	}
	else {
		spdlog::error("Failed to load mesh");
	}
}

void Mesh::SetMaterialIndex(uint32_t i) {
	m_MaterialIndex = i;
}

void Mesh::CalculateTriangles() {
	//spdlog::info("Triangles");
	//spdlog::info("{}", m_Indices.size());
	for (uint32_t i = 0; i < m_Indices.size(); i += 3)
	{
		Triangle triangle;
		triangle.A = m_Vertices[m_Indices[i]];
		triangle.B = m_Vertices[m_Indices[i + 1]];
		triangle.C = m_Vertices[m_Indices[i + 2]];
		m_Triangles.push_back(triangle);
		//spdlog::info("({}), ({}), ({})", triangle.A, triangle.B, triangle.C);
	}
}