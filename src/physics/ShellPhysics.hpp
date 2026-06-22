#pragma once

#include "../precompiled.h"
#include "../world/Terrain.hpp"

namespace ShellPhysics
{
	// Runtime state for the rolling shell. This is separated from Settings so
	// gameplay tuning can change without resetting velocity, spin, or contacts.
	struct State
	{
		// Linear movement in world space.
		glm::vec3 velocity{ 0.0f };
		// Angular velocity is derived from rolling speed and exposed for tuning.
		glm::vec3 angular_velocity{ 0.0f };
		// Visual rolling rotation applied to the shell mesh.
		glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };

		// Contact state used to switch between grounded friction and air control.
		bool grounded = true;
		bool was_grounded = true;

		// Timers stabilize launching, snapping, and bounce decisions.
		float air_time = 0.0f;
		float ground_contact_time = 0.0f;

		// Cached terrain normal from the latest contact/sample.
		glm::vec3 last_ground_normal{ 0.0f, 1.0f, 0.0f };
	};

	// Tunable constants for one shell-physics step. Most values are copied from
	// Config by Snail::BuildShellPhysicsSettings().
	struct Settings
	{
		// Collision radius and vertical clearance above the terrain.
		float radius = 0.0f;
		float ground_offset = 0.0f;

		float gravity = 0.0f;

		// Player control strength on ground and in air.
		float acceleration = 0.0f;
		float air_control = 0.0f;

		// Slope response. downhill_force makes hills matter; uphill_drag limits
		// unrealistic climbing.
		float downhill_force = 0.0f;
		float uphill_drag = 0.0f;

		// Damping values. They are raised to dt * 60 so tuning is stable across
		// frame rates.
		float ground_friction = 0.0f;
		float slope_friction = 0.0f;
		float air_drag = 0.0f;

		// Speed caps keep the simulation controllable.
		float max_speed = 0.0f;
		float max_vertical_speed = 0.0f;

		// Bounce parameters split normal and tangent energy after impact.
		float bounce_restitution = 0.0f;
		float bounce_tangent_keep = 0.0f;
		float min_bounce_speed = 0.0f;

		// Launch parameters allow the shell to leave the ground over hill crests.
		float launch_speed = 0.0f;
		float launch_normal_up_dot = 0.0f;
		float launch_boost = 0.0f;

		// Contact stabilization values.
		float snap_to_ground_distance = 0.0f;
		float sleep_vertical_epsilon = 0.0f;

		// Substeps reduce tunneling and make collision response less frame-rate
		// dependent.
		int substeps = 1;

		float world_bound = 0.0f;
	};

	// Horizontal speed is used for steering and rolling spin. Vertical velocity
	// is ignored so falling/bouncing does not change heading unexpectedly.
	inline glm::vec3 HorizontalVelocity(const State& state)
	{
		return { state.velocity.x, 0.0f, state.velocity.z };
	}

	// Keeps shell movement inside the terrain bounds and reflects velocity.
	void ClampToBounds(glm::vec3& position, glm::vec3& velocity, float bound, float restitution = -0.35f);

	// Clears velocity, rotation, and contact timers.
	void Reset(State& state);

	// Advances shell simulation by dt. The function mutates both State and the
	// owning snail position/yaw because shell mode drives the character anchor.
	void Step(
		State& state,
		glm::vec3& position,
		float& yaw,
		const glm::vec3& input,
		float dt,
		const Terrain& terrain,
		const Settings& settings
	);

	// Full 3D speed, useful for UI and vegetation collision effects.
	float GetSpeed(const State& state);
}
