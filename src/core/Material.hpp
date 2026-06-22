#pragma once
#include "../precompiled.h"
#include "Shader.hpp"
#include "Texture.hpp"

struct Material
{
	// Sends scalar material values and optional textures to the active shader.
	void Bind(const std::shared_ptr<Shader>& shader) const;
	// Unbinds texture slots used by Bind().
	void Unbind(const std::shared_ptr<Shader>& shader) const;

	// Name from the MTL file, useful for debugging and targeted material edits.
	const std::string name;

	// Base PBR-style parameters. Textures override or modulate these values.
	glm::vec3 diffuse{0.0f};
	glm::vec3 ambient{0.0f};
	float roughness{0.0f};
	float metallic{0.0f};
	float dissolve{ 1.0f };

	// Optional maps loaded from MTL references. The shader uses fixed texture
	// units, so Material::Bind must stay in sync with terrain.fragmentshader.
	std::shared_ptr<Texture> diffuse_texture = nullptr;
	std::shared_ptr<Texture> ao_texture = nullptr;
	std::shared_ptr<Texture> roughness_texture = nullptr;
	std::shared_ptr<Texture> metallic_texture = nullptr;
	std::shared_ptr<Texture> dissolve_texture = nullptr;
	std::shared_ptr<Texture> normal_texture = nullptr;
};
