#pragma once
#include "../precompiled.h"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Logger.hpp"

#include <tiny_obj_loader.h>

class Loader
{
public:
	// Load meshes + materials from an OBJ path relative to assets/models
	static void LoadModel(const std::string& path, 
		std::vector<std::shared_ptr<Mesh>>& meshes, 
		std::vector<std::shared_ptr<Material>>& materials);

	// Load standard 8-bit texture (png/jpg/etc) from assets/textures
	static std::shared_ptr<Texture> LoadTexture(const std::string& path);

	// Load HDR environment map from assets/hdr
	static std::shared_ptr<Texture> LoadEnvironment(const std::string& path);

	// Convenience utilities kept for compatibility with your version
	static std::shared_ptr<Material> GetMaterialByName(const std::string& name, 
		const std::vector<std::shared_ptr<Material>>& materials);


	static void UpdateMaterialDiffuseColor(const std::string& materialName, 
		const glm::vec3& newColor, std::vector<std::shared_ptr<Material>>& materials);

	static std::vector<std::shared_ptr<Material>> GetMaterials(const std::string& modelPath);

private:
	// Helpers (implementation details hidden in the .cpp)
	static void LoadMaterials(std::vector<std::shared_ptr<Material>>& materials, 
		const std::vector<tinyobj::material_t>& temp_materials);
	static void LoadMeshes(std::vector<std::shared_ptr<Mesh>>& meshes, 
		const std::vector<tinyobj::shape_t>& temp_shapes, const tinyobj::attrib_t& temp_attrib);

	// Cache of textures by relative path (assets/*)
	inline static std::unordered_map<std::string, std::shared_ptr<Texture>> unique_textures_{};
};
