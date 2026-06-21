#include "../precompiled.h"
#include "Loader.hpp"

#include "stb_image.h"


// -----------------------------
// small file-local helpers
// -----------------------------
namespace {
	[[noreturn]] void throwf(const std::string& msg, const std::string& path) {
		throw std::runtime_error(msg + ": " + path);
	}

	// normalize a relative path key for the cache (keeps your original key as given)
	std::string normalizeKey(const std::string& rel) {
		return rel;
	}
} 


// ============================================================================
// Model loading (OBJ via tinyobj)
// ============================================================================

void Loader::LoadModel(const std::string& path, std::vector<std::shared_ptr<Mesh>>& meshes, std::vector<std::shared_ptr<Material>>& materials)
{
	// Reject path traversal attempts
	if (path.find("..") != std::string::npos)
	{
		throwf("Model path rejected (path traversal)", path);
	}

	tinyobj::ObjReaderConfig reader_config;
	reader_config.vertex_color = false;
	reader_config.triangulation_method = "earcut";

	const auto model_path = std::filesystem::current_path() / "assets/models" / path;
	reader_config.mtl_search_path = model_path.parent_path().string();

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(model_path.string(), reader_config))
	{
		Logger::Log("TinyObj failed to parse model.", Logger::LogLevel::ERROR);
		Logger::Log("Input path: " + path, Logger::LogLevel::ERROR);
		Logger::Log("Resolved path: " + model_path.string(), Logger::LogLevel::ERROR);
		Logger::Log("TinyObj error: " + reader.Error(), Logger::LogLevel::ERROR);
		Logger::Log("TinyObj warning: " + reader.Warning(), Logger::LogLevel::WARNING);

		throwf("Failed to load model", path);
	}

	auto& temp_materials = reader.GetMaterials();
	auto& temp_attrib = reader.GetAttrib();
	auto& temp_shapes = reader.GetShapes();

	meshes.clear();
	materials.clear();

	LoadMaterials(materials, temp_materials);
	LoadMeshes(meshes, temp_shapes, temp_attrib);
}

// ============================================================================
// Textures (8-bit + HDR)
// ============================================================================
std::shared_ptr<Texture> Loader::LoadTexture(const std::string& path)
{
	if (path.empty())
		return nullptr;

	// Reject path traversal attempts
	if (path.find("..") != std::string::npos)
	{
		std::cerr << "Texture path rejected (path traversal): " << path << std::endl;
		return nullptr;
	}

	if (unique_textures_.contains(path))
		return unique_textures_[path];

	int channels, width, height;
	const auto image_path = std::filesystem::current_path() / "assets/textures" / path;

	if (!stbi_info(image_path.string().c_str(), &width, &height, &channels))
	{
		Logger::Log("Texture not found or cannot be decoded: " + image_path.string(), Logger::LogLevel::WARNING);
		return nullptr;
	}

	unsigned char* image_data = stbi_load(image_path.string().c_str(), &width, &height, &channels, 0);

	if (!image_data)
	{
		Logger::Log("stbi_load failed for image: " + image_path.string(), Logger::LogLevel::ERROR);
		return nullptr;
	}

	Logger::Log(
		"Loaded texture: " + image_path.string()
		+ " | channels=" + std::to_string(channels)
		+ " | size=" + std::to_string(width) + "x" + std::to_string(height)
	);

	const auto texture = std::make_shared<Texture>(image_data, width, height, channels);

	stbi_image_free(image_data);

	unique_textures_.insert({ path, texture });
	return texture;
}

std::shared_ptr<Texture> Loader::LoadEnvironment(const std::string& path)
{
	// Reject path traversal attempts
	if (path.find("..") != std::string::npos)
	{
		throwf("HDR path rejected (path traversal)", path);
	}

	int channels, width, height;
	const auto image_path = std::filesystem::current_path() / "assets/hdr" / path;

	stbi_set_flip_vertically_on_load(true);

	if (!stbi_info(image_path.string().c_str(), &width, &height, &channels)) {
		throwf("HDR cannot be found or decoded", path);
	}

	float* hdr_data = stbi_loadf(image_path.string().c_str(), &width, &height, &channels, 3);

	stbi_set_flip_vertically_on_load(false);
	if (!hdr_data) {
		throwf("stbi_loadf failed for HDR image", path);
	}

	const auto texture = std::make_shared<Texture>(hdr_data, width, height);

	stbi_image_free(hdr_data);

	return texture;
}

// ============================================================================
// Materials & Meshes (private helpers)
// ============================================================================
void Loader::LoadMaterials(
	std::vector<std::shared_ptr<Material>>& materials,
	const std::vector<tinyobj::material_t>& temp_materials
)
{
	materials.clear();

	if (temp_materials.empty())
	{
		materials.push_back(std::make_shared<Material>(
			"default",
			glm::vec3(0.75f),
			glm::vec3(1.0f),
			0.65f,
			0.0f,
			1.0f,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr
		));

		return;
	}

	for (const auto& material : temp_materials)
	{
		const std::string normalTexture =
			!material.normal_texname.empty()
			? material.normal_texname
			: material.bump_texname;

		const float roughness =
			material.roughness > 0.0f
			? material.roughness
			: 0.65f;

		const float dissolve =
			material.dissolve > 0.0f
			? material.dissolve
			: 1.0f;

		materials.push_back(std::make_shared<Material>(
			material.name,
			glm::vec3(
				material.diffuse[0],
				material.diffuse[1],
				material.diffuse[2]
			),
			glm::vec3(
				material.ambient[0],
				material.ambient[1],
				material.ambient[2]
			),
			roughness,
			material.metallic,
			dissolve,

			LoadTexture(material.diffuse_texname),
			LoadTexture(material.ambient_texname),
			LoadTexture(material.roughness_texname),
			LoadTexture(material.metallic_texname),
			LoadTexture(material.alpha_texname),
			LoadTexture(normalTexture)
		));
	}
}

void Loader::LoadMeshes(
	std::vector<std::shared_ptr<Mesh>>& meshes,
	const std::vector<tinyobj::shape_t>& temp_shapes,
	const tinyobj::attrib_t& temp_attrib
)
{
	struct MeshBuilder
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned> indices;
		std::unordered_map<Vertex, uint32_t> unique_vertices;
	};

	meshes.clear();

	for (const auto& shape : temp_shapes)
	{
		std::unordered_map<int, MeshBuilder> builders;

		size_t indexOffset = 0;

		for (size_t faceIndex = 0; faceIndex < shape.mesh.num_face_vertices.size(); ++faceIndex)
		{
			const int faceVertexCount = shape.mesh.num_face_vertices[faceIndex];

			if (faceVertexCount < 3)
			{
				indexOffset += faceVertexCount;
				continue;
			}

			int materialId = 0;

			if (faceIndex < shape.mesh.material_ids.size() &&
				shape.mesh.material_ids[faceIndex] >= 0)
			{
				materialId = shape.mesh.material_ids[faceIndex];
			}

			MeshBuilder& builder = builders[materialId];

			for (int v = 0; v < faceVertexCount; ++v)
			{
				const tinyobj::index_t index =
					shape.mesh.indices[indexOffset + static_cast<size_t>(v)];

				Vertex vertex{};

				if (index.vertex_index >= 0)
				{
					vertex.position =
					{
						temp_attrib.vertices[3 * index.vertex_index + 0],
						temp_attrib.vertices[3 * index.vertex_index + 1],
						temp_attrib.vertices[3 * index.vertex_index + 2]
					};
				}

				if (index.normal_index >= 0)
				{
					vertex.normal =
					{
						temp_attrib.normals[3 * index.normal_index + 0],
						temp_attrib.normals[3 * index.normal_index + 1],
						temp_attrib.normals[3 * index.normal_index + 2]
					};
				}
				else
				{
					vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
				}

				if (index.texcoord_index >= 0)
				{
					vertex.uv =
					{
						temp_attrib.texcoords[2 * index.texcoord_index + 0],
						temp_attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}
				else
				{
					vertex.uv = glm::vec2(0.0f);
				}

				if (!builder.unique_vertices.contains(vertex))
				{
					builder.unique_vertices[vertex] =
						static_cast<uint32_t>(builder.vertices.size());

					builder.vertices.push_back(vertex);
				}

				builder.indices.push_back(builder.unique_vertices[vertex]);
			}

			indexOffset += faceVertexCount;
		}

		for (auto& [materialId, builder] : builders)
		{
			if (!builder.vertices.empty() && !builder.indices.empty())
			{
				meshes.push_back(std::make_shared<Mesh>(
					builder.vertices,
					builder.indices,
					materialId
				));
			}
		}
	}
}

// ============================================================================
// Convenience wrappers 
// ============================================================================
std::vector<std::shared_ptr<Material>> Loader::GetMaterials(const std::string& modelPath)
{
	std::vector<std::shared_ptr<Material>> materials;
	std::vector<std::shared_ptr<Mesh>> meshes;

	// Load the model to extract its materials
	LoadModel(modelPath, meshes, materials);

	return materials;
}

std::shared_ptr<Material> Loader::GetMaterialByName(const std::string& name, const std::vector<std::shared_ptr<Material>>& materials)
{
	for (const auto& material : materials) {
		if (material->name == name) {
			return material;
		}
	}
	return nullptr; // Return nullptr if material is not found
}

void Loader::UpdateMaterialDiffuseColor(const std::string& materialName, const glm::vec3& newColor, std::vector<std::shared_ptr<Material>>& materials)
{
	auto material = GetMaterialByName(materialName, materials);
	if (material) {
		material->diffuse = newColor;
		Logger::Log("Updated material " + materialName + " diffuse color to: " + glm::to_string(newColor));
	}
	else {
		Logger::Log("Material " + materialName + " not found!", Logger::LogLevel::WARNING);
	}
}
