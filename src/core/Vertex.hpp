#pragma once
#include "../precompiled.h"
#include <glm/gtx/hash.hpp>

struct Vertex
{
	// Standard per-vertex data consumed by terrain.fragmentshader and model
	// shaders: position, lighting normal, UV, and material slot.
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
	int material_id;

	// Equality is used by Loader to de-duplicate OBJ vertices before uploading.
	bool operator==(const Vertex& other) const
	{
		return position == other.position
			&& normal == other.normal
			&& uv == other.uv;
	}
};

template<>
struct std::hash<Vertex>
{
	// Hash matches Vertex::operator== fields. material_id is intentionally not
	// included because the loader groups faces by material before building Mesh.
	size_t operator()(const Vertex& v) const noexcept
	{
		return std::hash<glm::vec3>()(v.position)
			^ std::hash<glm::vec3>()(v.normal)
			^ std::hash<glm::vec2>()(v.uv);
	}
};
