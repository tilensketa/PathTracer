#include "Model.h"

#include <spdlog/spdlog.h>

Model::Model(const std::string& path) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		spdlog::error("Error: {}", importer.GetErrorString());
	}
	else {
		// Start processing the model data beginning with the root node
		ProcessNode(scene->mRootNode, scene);
	}
	m_AABB = CreateAABB();
}

bool Model::IntersectsWithRay(const glm::vec3& origin, const glm::vec3& direction) const {
	return GetAABB().IntersectsWithRay(origin, direction);
}

void Model::ProcessNode(const aiNode* node, const aiScene* scene)
{

	for (uint32_t i = 0; i < node->mNumMeshes; i++)
	{

		uint32_t meshIndex = node->mMeshes[i];

		if (meshIndex < scene->mNumMeshes) {
			std::string name;
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			aiMesh* mesh = scene->mMeshes[meshIndex];

			// Access mesh information
			std::string meshName = mesh->mName.C_Str();
			uint32_t numVertices = mesh->mNumVertices;
			spdlog::info("----------------------------------------");
			spdlog::info("Mesh name: {}", meshName);
			spdlog::info("Number of vertices: {}", numVertices);
			name = meshName;

			for (uint32_t j = 0; j < mesh->mNumVertices; j++)
			{
				Vertex vert;

				aiVector3D vertex = mesh->mVertices[j];
				aiVector3D normal = mesh->mNormals[j];
				//spdlog::info("Vertex: ({}, {}, {})", vertex.x, vertex.y, vertex.z);
				//spdlog::info("Normal: ({}, {}, {})", normal.x, normal.y, normal.z);
				vert.Position = glm::vec3(vertex.x, vertex.y, vertex.z);
				vert.Normal = glm::vec3(normal.x, normal.y, normal.z);

				if (mesh->HasTextureCoords(0))
				{
					aiVector3D texCoord = mesh->mTextureCoords[0][j];
					//spdlog::info("TexCoord: ({}, {})", texCoord.x, texCoord.y);
					vert.TexCoord = glm::vec2(texCoord.x, texCoord.y);
				}
				vertices.push_back(vert);
			}

			// Access the indices to construct triangles.
			for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
				const aiFace& face = mesh->mFaces[j];
				if (face.mNumIndices != 3) {
					// Handle non-triangle faces if necessary.
				}
				else {
					indices.push_back(face.mIndices[0]);
					indices.push_back(face.mIndices[1]);
					indices.push_back(face.mIndices[2]);
				}
			}

			// Process material information for this mesh
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			// Access material properties
			Material newMaterial = ProcessNodeMaterials(material);

			Mesh newMesh(name, vertices, indices, newMaterial);
			m_Meshes.push_back(newMesh);
		}

	}
	// Recursively process child nodes
	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

void Model::ProcessMaterialTextures(aiMaterial* material, aiTextureType textureType, Material& outMaterial) {
	// Get the number of textures of the specified type in the material.
	unsigned int textureCount = material->GetTextureCount(textureType);

	if (textureCount > 0) {
		for (unsigned int i = 0; i < textureCount; ++i) {
			aiString texturePath;
			if (material->GetTexture(textureType, i, &texturePath) == AI_SUCCESS) {
				spdlog::info("Texture {}: {}", i + 1, texturePath.C_Str());
				outMaterial.Textures.push_back(Texture(texturePath.C_Str()));
				if (textureType == aiTextureType_DIFFUSE) {
					outMaterial.DiffuseTextureIndex = outMaterial.Textures.size() - 1;
				}
				else if (textureType == aiTextureType_SPECULAR) {
					outMaterial.SpecularTextureIndex = outMaterial.Textures.size() - 1;
				}
				else if (textureType == aiTextureType_NORMALS) {
					outMaterial.NormalTextureIndex = outMaterial.Textures.size() - 1;
				}
				else if (textureType == aiTextureType_SHININESS) {
					outMaterial.ShininessTextureIndex = outMaterial.Textures.size() - 1;
				}
			}
		}
	}
}

Material Model::ProcessNodeMaterials(aiMaterial* material) {

	Material newMaterial;

	spdlog::info("Material name: {}", material->GetName().C_Str());
	newMaterial.Name = material->GetName().C_Str();

	// Process various texture types.
	ProcessMaterialTextures(material, aiTextureType_DIFFUSE, newMaterial);
	ProcessMaterialTextures(material, aiTextureType_SPECULAR, newMaterial);
	ProcessMaterialTextures(material, aiTextureType_NORMALS, newMaterial);
	ProcessMaterialTextures(material, aiTextureType_SHININESS, newMaterial);

	// Access other material properties if required.
	aiColor3D ambientColor;
	if (material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == AI_SUCCESS) {
		spdlog::info("Ambient color: ({},{},{})", ambientColor.r, ambientColor.g, ambientColor.b);
		newMaterial.AmbientColor = glm::vec3(ambientColor.r, ambientColor.g, ambientColor.b);
	}
	aiColor3D diffuseColor;
	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
		spdlog::info("Diffuse color: ({},{},{})", diffuseColor.r, diffuseColor.g, diffuseColor.b);
		newMaterial.DiffuseColor = glm::vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b);
	}
	/*aiColor3D specularColor;
	if (material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
		spdlog::info("Specular color: ({},{},{})", specularColor.r, specularColor.g, specularColor.b);
		newMaterial.SpecularColor = glm::vec3(specularColor.r, specularColor.g, specularColor.b);
	}*/
	aiColor3D emissiveColor;
	if (material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) == AI_SUCCESS) {
		spdlog::info("Emissive color: ({},{},{})", emissiveColor.r, emissiveColor.g, emissiveColor.b);
		newMaterial.EmissionColor = glm::vec3(emissiveColor.r, emissiveColor.g, emissiveColor.b);
	}
	aiColor3D reflectiveColor;
	if (material->Get(AI_MATKEY_COLOR_REFLECTIVE, reflectiveColor) == AI_SUCCESS) {
		spdlog::info("Reflective color: ({},{},{})", reflectiveColor.r, reflectiveColor.g, reflectiveColor.b);
		newMaterial.ReflectiveColor = glm::vec3(reflectiveColor.r, reflectiveColor.g, reflectiveColor.b);
	}
	aiColor3D transparentColor;
	if (material->Get(AI_MATKEY_COLOR_TRANSPARENT, transparentColor) == AI_SUCCESS) {
		spdlog::info("Transparent color: ({},{},{})", transparentColor.r, transparentColor.g, transparentColor.b);
		newMaterial.TransparentColor = glm::vec3(transparentColor.r, transparentColor.g, transparentColor.b);
	}

	/*float metallic;
	if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
		spdlog::info("Metallic: {}", metallic);
		newMaterial.Metallic = metallic;
	}*/
	float shininess;
	if (material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
		spdlog::info("Shininess: {}", shininess);
		newMaterial.Shininess = shininess;
	}
	/*float roughness;
	if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
		spdlog::info("Roughness: {}", roughness);
		newMaterial.Roughness = roughness;
	}*/
	float specular;
	if (material->Get(AI_MATKEY_SPECULAR_FACTOR, specular) == AI_SUCCESS) {
		spdlog::info("Specular: {}", specular);
		newMaterial.Specular = specular;
	}
	/*float emissive;
	if (material->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissive) == AI_SUCCESS) {
		spdlog::info("Emissive: {}", emissive);
		newMaterial.EmissionPower = emissive;
	}*/
	return newMaterial;
}

AABB Model::CreateAABB() {
	float max = std::numeric_limits<float>::max();
	float min = std::numeric_limits<float>::min();
	glm::vec3 minVert = glm::vec3(max, max, max);
	glm::vec3 maxVert = glm::vec3(-max, -max, -max);
	for (uint32_t i = 0; i < m_Meshes.size(); i++)
	{
		const Mesh& mesh = m_Meshes[i];
		const AABB& meshAABB = mesh.GetAABB();
		minVert.x = std::min(minVert.x, meshAABB.Min.x);
		minVert.y = std::min(minVert.y, meshAABB.Min.y);
		minVert.z = std::min(minVert.z, meshAABB.Min.z);

		maxVert.x = std::max(maxVert.x, meshAABB.Max.x);
		maxVert.y = std::max(maxVert.y, meshAABB.Max.y);
		maxVert.z = std::max(maxVert.z, meshAABB.Max.z);
	}
	return AABB(maxVert, minVert);
}