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

	// Binds precomputed environment maps before scene objects are rendered.
	void Prepare() const;
	// Draws the HDR-derived skybox/background.
	void Draw(const std::shared_ptr<Shader>& background_shader) const;

private:
	// Creates framebuffer/renderbuffer resources used for cubemap captures.
	void CreateBuffers();
	// Creates the cube mesh used for skybox and cubemap rendering.
	void CreateCube();
	// Creates the fullscreen quad used for BRDF LUT generation.
	void CreateQuad();

	// Renders six cubemap faces for a target environment texture.
	void RenderCubeMapFaces(
		const Shader& shader,
		const glm::mat4 capture_views[6],
		GLuint targetTexture,
		int mipLevel = 0
	) const;

	// Converts the equirectangular HDR image into a cubemap.
	void RenderCubeMap(const glm::mat4& capture_projection, const glm::mat4 capture_views[6]) const;
	// Diffuse irradiance map for ambient lighting.
	void RenderIrradianceMap(const glm::mat4& capture_projection, const glm::mat4 capture_views[6]) const;
	// Specular prefiltered mip chain for roughness-dependent reflections.
	void RenderPrefilterMap(const glm::mat4& capture_projection, const glm::mat4 capture_views[6]) const;
	// 2D BRDF lookup table used by the PBR shader.
	void RenderBrdfLut() const;

	// Capture framebuffer resources.
	unsigned fbo_;
	unsigned rbo_;

	std::unique_ptr<Mesh> cube_ = nullptr;
	std::unique_ptr<Mesh> quad_ = nullptr;

	// Environment texture chain used by image-based lighting.
	std::shared_ptr<Texture> hdr_texture_ = nullptr;
	std::shared_ptr<Texture> cube_map_ = nullptr;
	std::shared_ptr<Texture> irradiance_map_ = nullptr;
	std::shared_ptr<Texture> prefilter_map_ = nullptr;
	std::shared_ptr<Texture> brdf_lut_ = nullptr;

	// Shaders used only during environment preparation.
	std::unique_ptr<Shader> cube_map_shader_ = nullptr;
	std::unique_ptr<Shader> brdf_shader_ = nullptr;
	std::unique_ptr<Shader> irradiance_shader_ = nullptr;
	std::unique_ptr<Shader> prefilter_shader_ = nullptr;
};
