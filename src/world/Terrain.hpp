#pragma once

#include "../precompiled.h"
#include "../core/Shader.hpp"
#include "../core/Vertex.hpp"

class Terrain
{
public:
	Terrain() = default;
	~Terrain();

	// Builds the terrain grid, fills vertices/indices, calculates normals, and
	// uploads the buffers to OpenGL.
	void Generate(int resolution, float size);
	// Renders the terrain in the main material pass.
	void Draw(const std::shared_ptr<Shader>& shader) const;
	// Renders the terrain into the directional-light shadow map.
	void DrawDepth(const std::shared_ptr<Shader>& shader) const;

	// Height and normal sampling used by the snail, shell physics, and
	// vegetation placement.
	float GetHeightAt(float x, float z) const;
	glm::vec3 GetNormalAt(float x, float z) const;

	void SetTexture(GLuint textureId);
	void SetModelMatrix(const glm::mat4& model);

	int GetResolution() const { return resolution_; }
	float GetSize() const { return size_; }
	float GetHalfSize() const { return half_size_; }

private:
	// Procedural height function. This is the single source of truth for both
	// generated mesh vertices and runtime physics queries.
	float GetProceduralHeight(float x, float z) const;
	// Averages triangle normals so terrain lighting is smooth instead of faceted.
	void RecalculateNormals();
	// Creates per-tile UV rotation/flips to reduce visible texture repetition.
	glm::vec2 TransformTileUV(glm::vec2 uv, int tileX, int tileZ) const;

private:
	// OpenGL mesh resources for the generated terrain grid.
	GLuint vao_ = 0;
	GLuint vbo_ = 0;
	GLuint ebo_ = 0;

	// Texture id for the grass/dirt terrain material.
	GLuint texture_id_ = 0;

	// Grid settings. resolution_ is the number of cells per side; vertices are
	// generated as (resolution_ + 1)^2.
	int resolution_ = 0;
	float size_ = 0.0f;
	float half_size_ = 0.0f;

	// World transform, currently identity but kept for future terrain placement.
	glm::mat4 model_ = glm::mat4(1.0f);

	std::vector<Vertex> vertices_;
	std::vector<unsigned int> indices_;
};
