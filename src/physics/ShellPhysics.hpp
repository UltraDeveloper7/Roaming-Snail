#pragma once

#include "../precompiled.h"
#include "../world/Terrain.hpp"

namespace ShellPhysics
{
	struct State
	{
		glm::vec3 velocity{ 0.0f };
		glm::vec3 angular_velocity{ 0.0f };
		glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };

		bool grounded = true;
		bool was_grounded = true;

		float air_time = 0.0f;
		float ground_contact_time = 0.0f;

		glm::vec3 last_ground_normal{ 0.0f, 1.0f, 0.0f };
	};

	struct Settings
	{
		float radius = 0.0f;
		float ground_offset = 0.0f;

		float gravity = 0.0f;

		float acceleration = 0.0f;
		float air_control = 0.0f;

		float downhill_force = 0.0f;
		float uphill_drag = 0.0f;

		float ground_friction = 0.0f;
		float slope_friction = 0.0f;
		float air_drag = 0.0f;

		float max_speed = 0.0f;
		float max_vertical_speed = 0.0f;

		float bounce_restitution = 0.0f;
		float bounce_tangent_keep = 0.0f;
		float min_bounce_speed = 0.0f;

		float launch_speed = 0.0f;
		float launch_normal_up_dot = 0.0f;
		float launch_boost = 0.0f;

		float snap_to_ground_distance = 0.0f;
		float sleep_vertical_epsilon = 0.0f;

		int substeps = 1;

		float world_bound = 0.0f;
	};

	inline glm::vec3 HorizontalVelocity(const State& state)
	{
		return { state.velocity.x, 0.0f, state.velocity.z };
	}

	void ClampToBounds(glm::vec3& position, glm::vec3& velocity, float bound, float restitution = -0.35f);

	void Reset(State& state);

	void Step(
		State& state,
		glm::vec3& position,
		float& yaw,
		const glm::vec3& input,
		float dt,
		const Terrain& terrain,
		const Settings& settings
	);

	float GetSpeed(const State& state);
}