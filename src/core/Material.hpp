#pragma once
#include "../precompiled.h"
#include "Shader.hpp"
#include "Texture.hpp"

struct Material
{
	void Bind(const std::shared_ptr<Shader>& shader) const;
	void Unbind(const std::shared_ptr<Shader>& shader) const;

	const std::string name;

	// Base parameters
	glm::vec3 diffuse{0.0f};	// baseColor if no map
	glm::vec3 ambient{0.0f};	// used as AO (shader reads .r)
	float roughness{0.0f};
	float metallic{0.0f};
	float dissolve{ 1.0f };		// alpha

	// Optional maps (fixed texture units set once in App::Load)
	std::shared_ptr<Texture> diffuse_texture = nullptr;
	std::shared_ptr<Texture> ao_texture = nullptr;
	std::shared_ptr<Texture> roughness_texture = nullptr;
	std::shared_ptr<Texture> metallic_texture = nullptr;
	std::shared_ptr<Texture> dissolve_texture = nullptr;	// (not used by shader
	std::shared_ptr<Texture> normal_texture = nullptr;
};
