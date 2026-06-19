#include "../precompiled.h"
#include "ShellPhysics.hpp"

namespace
{
	glm::vec3 ProjectOnPlane(const glm::vec3& value, const glm::vec3& normal)
	{
		return value - normal * glm::dot(value, normal);
	}

	void ClampVelocity(
		ShellPhysics::State& state,
		const ShellPhysics::Settings& settings
	)
	{
		const glm::vec2 horizontalVelocity(state.velocity.x, state.velocity.z);
		const float horizontalSpeed = glm::length(horizontalVelocity);

		if (horizontalSpeed > settings.max_speed && horizontalSpeed > 0.0001f)
		{
			const glm::vec2 clamped =
				glm::normalize(horizontalVelocity) * settings.max_speed;

			state.velocity.x = clamped.x;
			state.velocity.z = clamped.y;
		}

		state.velocity.y = glm::clamp(
			state.velocity.y,
			-settings.max_vertical_speed,
			settings.max_vertical_speed
		);
	}

	bool ShouldLaunchFromHill(
		const ShellPhysics::State& state,
		const ShellPhysics::Settings& settings,
		const glm::vec3& terrainNormal
	)
	{
		const glm::vec3 hv = ShellPhysics::HorizontalVelocity(state);

		const float speed = glm::length(hv);

		if (speed < settings.launch_speed)
		{
			return false;
		}

		const float upDot =
			glm::dot(terrainNormal, glm::vec3(0.0f, 1.0f, 0.0f));

		if (upDot < settings.launch_normal_up_dot)
		{
			return false;
		}

		if (state.ground_contact_time < 0.08f)
		{
			return false;
		}

		return true;
	}

	void UpdateSpin(
		ShellPhysics::State& state,
		float dt,
		const ShellPhysics::Settings& settings
	)
	{
		const glm::vec3 hv = ShellPhysics::HorizontalVelocity(state);

		const float speed = glm::length(hv);

		if (speed < 0.001f)
		{
			state.angular_velocity = glm::vec3(0.0f);
			return;
		}

		const glm::vec3 velocityDirection = glm::normalize(hv);

		glm::vec3 axis =
			glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), velocityDirection);

		if (glm::length2(axis) < 0.0001f)
		{
			state.angular_velocity = glm::vec3(0.0f);
			return;
		}

		axis = glm::normalize(axis);

		const float angularSpeed =
			speed / glm::max(settings.radius, 0.01f);

		const float angle = angularSpeed * dt;

		state.rotation =
			glm::normalize(glm::angleAxis(angle, axis) * state.rotation);

		state.angular_velocity = axis * angularSpeed;
	}

	void ResolveTerrainCollision(
		ShellPhysics::State& state,
		glm::vec3& position,
		const Terrain& terrain,
		const ShellPhysics::Settings& settings
	)
	{
		const float groundHeight =
			terrain.GetHeightAt(position.x, position.z);

		const glm::vec3 terrainNormal =
			glm::normalize(terrain.GetNormalAt(position.x, position.z));

		const float targetY =
			groundHeight + settings.radius + settings.ground_offset;

		const float penetration = targetY - position.y;

		state.was_grounded = state.grounded;

		if (penetration > 0.0f)
		{
			position.y = targetY;

			const float normalSpeed =
				glm::dot(state.velocity, terrainNormal);

			const glm::vec3 normalVelocity =
				terrainNormal * normalSpeed;

			const glm::vec3 tangentVelocity =
				state.velocity - normalVelocity;

			if (normalSpeed < -settings.min_bounce_speed)
			{
				state.velocity =
					tangentVelocity * settings.bounce_tangent_keep -
					normalVelocity * settings.bounce_restitution;

				state.grounded = false;
				state.air_time = 0.0f;
				state.ground_contact_time = 0.0f;
			}
			else
			{
				state.velocity = tangentVelocity;

				if (std::abs(state.velocity.y) < settings.sleep_vertical_epsilon)
				{
					state.velocity.y = 0.0f;
				}

				state.grounded = true;
				state.air_time = 0.0f;
			}

			state.last_ground_normal = terrainNormal;
			return;
		}

		const float distanceAboveGround = -penetration;

		if (distanceAboveGround > settings.snap_to_ground_distance)
		{
			state.grounded = false;
			return;
		}

		if (state.velocity.y <= 0.0f && state.ground_contact_time > 0.05f)
		{
			position.y = targetY;
			state.velocity = ProjectOnPlane(state.velocity, terrainNormal);
			state.grounded = true;
			state.air_time = 0.0f;
			state.last_ground_normal = terrainNormal;
		}
	}

	void IntegrateSubstep(
		ShellPhysics::State& state,
		glm::vec3& position,
		const glm::vec3& input,
		float dt,
		const Terrain& terrain,
		const ShellPhysics::Settings& settings
	)
	{
		const glm::vec3 terrainNormal =
			glm::normalize(terrain.GetNormalAt(position.x, position.z));

		state.last_ground_normal = terrainNormal;

		const glm::vec3 gravity(0.0f, -settings.gravity, 0.0f);

		if (state.grounded)
		{
			state.air_time = 0.0f;
			state.ground_contact_time += dt;

			const glm::vec3 downhill =
				ProjectOnPlane(gravity, terrainNormal);

			state.velocity += downhill * settings.downhill_force * dt;

			glm::vec3 inputWorld(input.x, 0.0f, input.z);
			inputWorld = ProjectOnPlane(inputWorld, terrainNormal);

			if (glm::length2(inputWorld) > 0.0001f)
			{
				inputWorld = glm::normalize(inputWorld);
				state.velocity += inputWorld * settings.acceleration * dt;
			}

			state.velocity = ProjectOnPlane(state.velocity, terrainNormal);

			const float upDot =
				glm::dot(terrainNormal, glm::vec3(0.0f, 1.0f, 0.0f));

			const float slopeAmount = glm::clamp(1.0f - upDot, 0.0f, 1.0f);

			const float friction =
				glm::mix(settings.ground_friction, settings.slope_friction, slopeAmount);

			state.velocity *= std::pow(friction, dt * 60.0f);

			if (ShouldLaunchFromHill(state, settings, terrainNormal))
			{
				state.grounded = false;
				state.air_time = 0.0f;
				state.ground_contact_time = 0.0f;
				state.velocity += terrainNormal * settings.launch_boost;
			}
		}
		else
		{
			state.air_time += dt;

			state.velocity += gravity * dt;

			glm::vec3 inputWorld(input.x, 0.0f, input.z);

			if (glm::length2(inputWorld) > 0.0001f)
			{
				inputWorld = glm::normalize(inputWorld);
				state.velocity += inputWorld * settings.air_control * dt;
			}

			state.velocity *= std::pow(settings.air_drag, dt * 60.0f);
		}

		position += state.velocity * dt;

		ShellPhysics::ClampToBounds(position, state.velocity, settings.world_bound);

		ResolveTerrainCollision(state, position, terrain, settings);
		ClampVelocity(state, settings);
	}
}

namespace ShellPhysics
{
	void ClampToBounds(glm::vec3& position, glm::vec3& velocity, float bound, float restitution)
	{
		const bool hitX = position.x <= -bound || position.x >= bound;
		const bool hitZ = position.z <= -bound || position.z >= bound;

		position.x = glm::clamp(position.x, -bound, bound);
		position.z = glm::clamp(position.z, -bound, bound);

		if (hitX) velocity.x *= restitution;
		if (hitZ) velocity.z *= restitution;
	}

	void Reset(State& state)
	{
		state.velocity = glm::vec3(0.0f);
		state.angular_velocity = glm::vec3(0.0f);
		state.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

		state.grounded = true;
		state.was_grounded = true;

		state.air_time = 0.0f;
		state.ground_contact_time = 0.0f;

		state.last_ground_normal = glm::vec3(0.0f, 1.0f, 0.0f);
	}

	void Step(
		State& state,
		glm::vec3& position,
		float& yaw,
		const glm::vec3& input,
		float dt,
		const Terrain& terrain,
		const Settings& settings
	)
	{
		dt = glm::clamp(dt, 0.0f, 1.0f / 20.0f);

		const int substeps = glm::max(1, settings.substeps);
		const float subDt = dt / static_cast<float>(substeps);

		for (int i = 0; i < substeps; ++i)
		{
			IntegrateSubstep(
				state,
				position,
				input,
				subDt,
				terrain,
				settings
			);
		}

		const glm::vec3 hv = HorizontalVelocity(state);

		if (glm::length2(hv) > 0.01f)
		{
			const glm::vec3 direction = glm::normalize(hv);
			yaw = std::atan2(direction.x, -direction.z);
		}

		UpdateSpin(state, dt, settings);
	}

	float GetSpeed(const State& state)
	{
		return glm::length(state.velocity);
	}
}