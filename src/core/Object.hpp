#pragma once
#include "../precompiled.h"
#include "../core/Material.hpp"
#include "../core/Mesh.hpp"
#include "../core/Shader.hpp"
#include "Logger.hpp"

class Object
{
public:
	virtual ~Object() = default;
	Object() = default;
	// Loads an OBJ model from assets/models through Loader.
	explicit Object(const std::string& path);

	// Draws using this object's own transform fields.
	virtual void Draw(const std::shared_ptr<Shader>& shader);
	// Draws using an externally supplied model matrix. Most instanced systems
	// use this path so one mesh can appear at many positions.
	void DrawWithModelMatrix(const std::shared_ptr<Shader>& shader, const glm::mat4& modelMatrix);
	// Depth-only rendering for opaque meshes.
	void DrawDepthWithModelMatrix(
		const std::shared_ptr<Shader>& shader,
		const glm::mat4& modelMatrix
	);
	// Depth rendering path for alpha-tested geometry such as vegetation cards.
	void DrawAlphaDepthWithModelMatrix(
		const std::shared_ptr<Shader>& shader,
		const glm::mat4& modelMatrix
	);
	// Basic transform mutators used by older/simple object code.
	void Translate(const glm::vec3& translation);
	void Scale(const glm::vec3& scale);
	void Rotate(const glm::vec3& rotation_axis, float angle);

	// Local transform state for Object::Draw().
	glm::vec3 translation_{ 0.0f, 0.0f, 0.0f };
	glm::vec3 scale_{ 1.0f, 1.0f, 1.0f };
	glm::vec3 rotation_axis_{ 0.0f, 0.0f, 0.0f };
	float angle_{ 0.0f };

	[[nodiscard]] glm::mat4 GetModelMatrix() const;

	// Setters allow procedural systems to build or replace model data.
	void SetMeshes(const std::vector<std::shared_ptr<Mesh>>& meshes) { meshes_ = meshes; }
	void SetMaterials(const std::vector<std::shared_ptr<Material>>& materials) { materials_ = materials; }

	// Accessors are useful for debugging, material inspection, and future tools.
	const std::vector<std::shared_ptr<Mesh>>& GetMeshes() const {
		return meshes_;
	}
	const std::vector<std::shared_ptr<Material>>& GetMaterials() const {
		return materials_;
	}

	bool HasValidMesh() const;

protected:
	// Material and mesh arrays keep the OBJ's material ids aligned with Mesh.
	std::vector<std::shared_ptr<Material>> materials_{};
	std::vector<std::shared_ptr<Mesh>> meshes_{}; 
};
