#pragma once

#include "Mesh.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <string>
#include <vector>

class Model {
public:
	Model(const std::string& path);

	const std::vector<Mesh>& GetMeshes() const { return m_Meshes; }
	std::vector<Mesh>& GetMeshes() { return m_Meshes; }
private:
	void ProcessNode(const aiNode* node, const aiScene* scene);
	void ProcessMaterialTextures(aiMaterial* material, aiTextureType textureType, Material& outMaterial);
	Material ProcessNodeMaterials(aiMaterial* material);
private:
	std::vector<Mesh> m_Meshes;
};