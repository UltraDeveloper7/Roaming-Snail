#pragma once
#include "../precompiled.h"
#include "../core/Shader.hpp"
#include "../core/Mesh.hpp"
#include "../core/Texture.hpp"

class Environment final
{
public:
	Environment();
	~Environment();

	// Draws the HDR-derived skybox/background.
	void Draw(const std::shared_ptr<Shader>& background_shader) const;

private:
	// Creates framebuffer/renderbuffer resources used for cubemap captures.
	void CreateBuffers();
	// Creates the cube mesh used for skybox and cubemap rendering.
	void CreateCube();

	// Renders six cubemap faces for a target environment texture.
	void RenderCubeMapFaces(
		const Shader& shader,
		const glm::mat4 capture_views[6],
		GLuint targetTexture,
		int mipLevel = 0
	) const;

	// Converts the equirectangular HDR image into a cubemap.
	void RenderCubeMap(const glm::mat4& capture_projection, const glm::mat4 capture_views[6]) const;

	// Capture framebuffer resources.
	unsigned fbo_;
	unsigned rbo_;

	std::unique_ptr<Mesh> cube_ = nullptr;

	// HDR source and cubemap used by the visible skybox.
	std::shared_ptr<Texture> hdr_texture_ = nullptr;
	std::shared_ptr<Texture> cube_map_ = nullptr;

	// Shader used only to convert the HDR panorama to a cubemap.
	std::unique_ptr<Shader> cube_map_shader_ = nullptr;
};
