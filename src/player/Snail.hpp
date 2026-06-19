#pragma once

#include "../precompiled.h"
#include "../core/Object.hpp"
#include "../core/Shader.hpp"
#include "../world/Terrain.hpp"
#include "../physics/ShellPhysics.hpp"

class Snail
{
public:
	Snail() = default;
	~Snail();

	void Init();

	void Update(float dt, GLFWwindow* window, const Terrain& terrain);

	void Draw(const std::shared_ptr<Shader>& shader) const;
	void DrawDepth(const std::shared_ptr<Shader>& shader) const;

	const glm::vec3& GetPosition() const { return position_; }

	bool IsShellMode() const
	{
		return mode_ == Mode::Shell;
	}

	void ApplySpeedBoost(float duration, float multiplier);
	void SlowShell(float multiplier);

	float GetShellSpeed() const
	{
		return ShellPhysics::GetSpeed(shell_physics_);
	}

	bool IsShellGrounded() const
	{
		return shell_physics_.grounded;
	}

private:
	enum class Mode
	{
		Normal,
		Retracting,
		Shell,
		Unretracting
	};

	void CreateBoxMesh();

	void HandleInput(float dt, GLFWwindow* window);

	void UpdateNormalMovement(float dt, GLFWwindow* window);
	void UpdateShellPhysics(float dt, GLFWwindow* window, const Terrain& terrain);
	void UpdateRetractAnimation(float dt);

	glm::vec3 ReadShellInput(GLFWwindow* window) const;
	ShellPhysics::Settings BuildShellPhysicsSettings() const;

	void ClampToTerrainBounds();
	void UpdateOrientationFromTerrain(const Terrain& terrain);

	float GetTerrainSlopeAngle(const Terrain& terrain) const;

	glm::mat4 ComputeBodyModelMatrix() const;
	glm::mat4 ComputeShellModelMatrix() const;

	glm::mat4 BuildModelMatrix(
		const glm::vec3& localOffset,
		const glm::vec3& localScale
	) const;

	bool IsBodyVisible() const;
	bool IsShellOnly() const;

private:
	glm::vec3 position_{ 0.0f };
	glm::vec3 forward_{ 0.0f, 0.0f, -1.0f };
	glm::vec3 right_{ 1.0f, 0.0f, 0.0f };
	glm::vec3 up_{ 0.0f, 1.0f, 0.0f };

	glm::mat4 orientation_{ 1.0f };

	float yaw_ = 0.0f;

	float move_speed_ = Config::snail_move_speed;
	float turn_speed_ = Config::snail_turn_speed;

	float animation_time_ = 0.0f;
	float creep_amount_ = 0.0f;
	bool is_moving_ = false;

	Mode mode_ = Mode::Normal;

	float retract_progress_ = 0.0f;
	float retract_speed_ = Config::snail_retract_speed;
	bool r_was_pressed_ = false;

	ShellPhysics::State shell_physics_;

	float shell_radius_ = Config::snail_shell_radius;
	float shell_turn_strength_ = Config::shell_turn_strength;
	float max_shell_climb_slope_ = Config::shell_max_climb_slope;

	float speed_boost_timer_ = 0.0f;
	float speed_boost_multiplier_ = 1.0f;

	GLuint vao_ = 0;
	GLuint vbo_ = 0;
	GLuint ebo_ = 0;

	std::vector<float> vertices_;
	std::vector<unsigned int> indices_;

	std::unique_ptr<Object> slug_object_ = nullptr;
	std::unique_ptr<Object> shell_object_ = nullptr;
};