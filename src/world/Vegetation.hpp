#pragma once

#include "../precompiled.h"
#include "../core/Shader.hpp"
#include "../core/Object.hpp"
#include "../world/Terrain.hpp"

class Vegetation
{
public:
	struct Plant
	{
		glm::vec3 position{ 0.0f };

		float scale = 1.0f;
		float rotation = 0.0f;
		glm::vec2 lean{ 0.0f };
		float bend = 0.0f;

		glm::vec3 nonUniformScale{ 1.0f };
		glm::vec3 colorTint{ 1.0f };

		bool eaten = false;
	};

public:
	Vegetation() = default;
	~Vegetation() = default;

	void Init();
	void Generate(const Terrain& terrain, int count, float terrainBound);
	void Update(float dt);
	void Draw(const std::shared_ptr<Shader>& shader) const;
	void DrawDepth(const std::shared_ptr<Shader>& shader) const;

	bool TryEat(const glm::vec3& snailPosition, float radius);
	bool TryHitShell(const glm::vec3& shellPosition, float radius);

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
	glm::mat4 BuildPlantModel(const Plant& plant) const;

private:
	std::unique_ptr<Object> plant_object_ = nullptr;
	std::vector<Plant> plants_;
};