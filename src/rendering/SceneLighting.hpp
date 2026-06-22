#pragma once

#include "../precompiled.h"
#include "../core/Shader.hpp"
#include "../Config.hpp"

class SceneLighting final
{
public:
	SceneLighting() = default;
	~SceneLighting();

	// Allocates shadow map resources and computes the initial sun projection.
	void Init();

	// Starts the depth-only render pass from the sun's point of view.
	void BeginShadowPass();
	// Restores the main viewport after the shadow pass.
	void EndShadowPass(int viewportWidth, int viewportHeight);

	// Sends directional light, shadow map, and light-space matrix to a scene
	// shader before drawing lit objects.
	void BindForScene(
		const std::shared_ptr<Shader>& shader,
		const glm::vec3& cameraPosition
	) const;

	const glm::mat4& GetLightSpaceMatrix() const { return light_space_matrix_; }
	GLuint GetShadowMap() const { return shadow_map_; }

private:
	// Creates the depth texture and framebuffer used for shadow mapping.
	void CreateShadowResources();
	// Builds the orthographic sun camera used for directional shadows.
	void UpdateLightSpaceMatrix();

private:
	// OpenGL resources for the shadow pass.
	GLuint shadow_fbo_ = 0;
	GLuint shadow_map_ = 0;

	// Projection * view matrix from the light's perspective.
	glm::mat4 light_space_matrix_{ 1.0f };
};
