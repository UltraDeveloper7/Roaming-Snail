#pragma once
#include "../precompiled.h"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Logger.hpp"

#include <tiny_obj_loader.h>

class Loader
{
public:
	// Loads meshes and materials from an OBJ path relative to assets/models.
	// Output vectors are cleared and refilled, so callers receive a complete
	// self-contained model representation.
	static void LoadModel(const std::string& path, 
		std::vector<std::shared_ptr<Mesh>>& meshes, 
		std::vector<std::shared_ptr<Material>>& materials);

	// Loads a standard 8-bit texture (png/jpg/etc) from assets/textures.
	static std::shared_ptr<Texture> LoadTexture(const std::string& path);

	// Loads an HDR environment map from assets/hdr.
	static std::shared_ptr<Texture> LoadEnvironment(const std::string& path);

	// Finds a material by name after model loading. Used for targeted tuning or
	// debug changes without reloading the OBJ.
	static std::shared_ptr<Material> GetMaterialByName(const std::string& name, 
		const std::vector<std::shared_ptr<Material>>& materials);

	// Runtime material edit helper, useful for quick visual experiments.
	static void UpdateMaterialDiffuseColor(const std::string& materialName, 
		const glm::vec3& newColor, std::vector<std::shared_ptr<Material>>& materials);

	// Convenience wrapper that loads only the material list for a model.
	static std::vector<std::shared_ptr<Material>> GetMaterials(const std::string& modelPath);

private:
	// Converts tinyobj materials into the engine Material structure and resolves
	// any texture file references.
	static void LoadMaterials(std::vector<std::shared_ptr<Material>>& materials, 
		const std::vector<tinyobj::material_t>& temp_materials);
	// Converts tinyobj shapes into GPU-ready Mesh objects with de-duplicated
	// vertices.
	static void LoadMeshes(std::vector<std::shared_ptr<Mesh>>& meshes, 
		const std::vector<tinyobj::shape_t>& temp_shapes, const tinyobj::attrib_t& temp_attrib);

	// Cache of textures by relative path. This prevents every object instance
	// from uploading duplicate OpenGL textures.
	inline static std::unordered_map<std::string, std::shared_ptr<Texture>> unique_textures_{};
};
