#pragma once

#include "../precompiled.h"
#include "../core/Shader.hpp"
#include "../core/Object.hpp"
#include "../world/Terrain.hpp"

class Vegetation
{
public:
	// One placed bush instance. The same OBJ mesh is reused for all plants, and
	// these values create variety through transform and color tint.
	struct Plant
	{
		// Terrain-snapped world position.
		glm::vec3 position{ 0.0f };

		// Base scale and random yaw rotation.
		float scale = 1.0f;
		float rotation = 0.0f;
		// Static lean makes plants feel naturally uneven.
		glm::vec2 lean{ 0.0f };
		// Temporary bend applied after shell collision.
		float bend = 0.0f;

		// Instance variation without needing extra meshes or materials.
		glm::vec3 nonUniformScale{ 1.0f };
		glm::vec3 colorTint{ 1.0f };

		// Eaten plants are skipped in rendering and collision.
		bool eaten = false;
	};

public:
	Vegetation() = default;
	~Vegetation() = default;

	// Loads the shared vegetation OBJ.
	void Init();
	// Deterministically scatters plants on the terrain, using height sampling so
	// every plant sits on the surface.
	void Generate(const Terrain& terrain, int count, float terrainBound);
	// Returns bent plants to their relaxed pose.
	void Update(float dt);
	// Draws all non-eaten plants in the main pass.
	void Draw(const std::shared_ptr<Shader>& shader) const;
	// Draws all non-eaten plants into the shadow map.
	void DrawDepth(const std::shared_ptr<Shader>& shader) const;

	// Normal snail interaction: consume a nearby plant and trigger a speed boost.
	bool TryEat(const glm::vec3& snailPosition, float radius);
	// Shell interaction: bend nearby plants and let the caller slow the shell.
	bool TryHitShell(const glm::vec3& shellPosition, float radius);

	// Shared radial query helper for eating and shell hits. The callback returns
	// true when the search should stop immediately.
	template<typename Func>
	bool ForEachPlantInRadius(const glm::vec3& position, float radius, Func func)
	{
		bool any = false;
		const glm::vec2 posXZ{ position.x, position.z };

		for (Plant& plant : plants_)
		{
			if (plant.eaten) continue;

			const glm::vec2 plantXZ{ plant.position.x, plant.position.z };
			if (glm::distance(posXZ, plantXZ) <= radius)
			{
				if (func(plant)) return true;
				any = true;
			}
		}
		return any;
	}

private:
	// Combines position, random rotation, lean, hit bend, and scale into a final
	// model matrix for one plant instance.
	glm::mat4 BuildPlantModel(const Plant& plant) const;

private:
	// Shared model loaded once and instanced by changing the model matrix.
	std::unique_ptr<Object> plant_object_ = nullptr;
	// All placed plant instances.
	std::vector<Plant> plants_;
};
